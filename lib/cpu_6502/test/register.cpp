#include "base_test.hpp"
#include <gtest/gtest.h>
#include <optional>

namespace emu::cpu6502 {

using namespace emu::cpu6502::opcode;

class RegisterBaseTest : public BaseTest {
public:
    void SetUp() override {
        random_reg_values = true;
        BaseTest::SetUp();
    }
};

using RegIncDecTestArg = std::tuple<Opcode, const char *, Reg8Ptr, bool>;
class RegIncDecTest : public RegisterBaseTest, public ::testing::WithParamInterface<RegIncDecTestArg> {
public:
    // Affect Flags: N Z
    // These instructions are implied mode, have a length of one byte and require two machine cycles.

    Reg8Ptr test_register;
    std::optional<uint8_t> result_byte;

    void Execute(const std::vector<uint8_t> &data) override {
        expected_regs.SetFlag(Flags::Zero, result_byte.value() == 0);
        expected_regs.SetFlag(Flags::Negative, (result_byte.value() & 0x80) > 0);
        RegisterBaseTest::Execute(data);
    }
};

TEST_P(RegIncDecTest, ) {
    auto [opcode, name, reg, inc] = GetParam();
    test_register = reg;
    expected_code_length = 1;
    // expected_cycles = 2;
    if (inc) {
        result_byte = (expected_regs.*reg) += 1;
    } else {
        result_byte = (expected_regs.*reg) -= 1;
    }
    Execute(MakeCode(opcode));
}

std::vector<RegIncDecTestArg> GetIncDecTestCases() {
    return {
        {INS_INX, "INX", &Registers::x, true},  //
        {INS_DEX, "DEX", &Registers::x, false}, //
        {INS_INY, "INY", &Registers::y, true},  //
        {INS_DEY, "DEY", &Registers::y, false}, //
        // {INS_INC_ACC, "INC_ACC", &Registers::a, true},  //
        // {INS_DEC_ACC, "DEC_ACC", &Registers::a, false}, //
    };
}

INSTANTIATE_TEST_SUITE_P(, RegIncDecTest, ::testing::ValuesIn(GetIncDecTestCases()), GenTestNameFunc());

using RegTransferTestArg = std::tuple<Opcode, const char *, Reg8Ptr, Reg8Ptr>;
class RegisterTransferTest : public RegisterBaseTest, public ::testing::WithParamInterface<RegTransferTestArg> {
public:
    // Affect Flags: N Z
    // These instructions are implied mode, have a length of one byte and require two machine cycles.

    Reg8Ptr source_reg;
    Reg8Ptr target_reg;

    void Execute(const std::vector<uint8_t> &data) override {
        auto source_byte = expected_regs.*source_reg;
        if (target_reg != &Registers::stack_pointer) {
            expected_regs.SetFlag(Flags::Zero, source_byte == 0);
            expected_regs.SetFlag(Flags::Negative, (source_byte & 0x80) > 0);
        }
        expected_regs.*target_reg = source_byte;
        RegisterBaseTest::Execute(data);
    }
};

TEST_P(RegisterTransferTest, ) {
    auto [opcode, name, src, dst] = GetParam();
    source_reg = src;
    target_reg = dst;
    expected_code_length = 1;
    // expected_cycles = 2;
    Execute(MakeCode(opcode));
}

std::vector<RegTransferTestArg> GetTransferTestCases() {
    return {
        {INS_TAX, "TAX", &Registers::a, &Registers::x},             //
        {INS_TAY, "TAY", &Registers::a, &Registers::y},             //
        {INS_TXA, "TXA", &Registers::x, &Registers::a},             //
        {INS_TYA, "TYA", &Registers::y, &Registers::a},             //
        {INS_TXS, "TXS", &Registers::x, &Registers::stack_pointer}, //
        {INS_TSX, "TSX", &Registers::stack_pointer, &Registers::x}, //
    };
}

INSTANTIATE_TEST_SUITE_P(, RegisterTransferTest, ::testing::ValuesIn(GetTransferTestCases()), GenTestNameFunc());

using FlagChangeTestArg = std::tuple<Opcode, std::string, Registers::Flags, bool>;

class FlagChangeTest : public RegisterBaseTest, public ::testing::WithParamInterface<FlagChangeTestArg> {
public:
    // Flag (Processor Status) Instructions
    // Affect Flags: as noted

    // These instructions are implied mode, have a length of one byte and require two machine cycles.

    // Notes:
    // The Interrupt flag is used to prevent (SEI) or enable (CLI) maskable interrupts
    // (aka IRQ's). It does not signal the presence or absence of an interrupt condition.
    // The 6502 will set this flag automatically in response to an interrupt and restore
    // it to its prior status on completion of the interrupt service routine. If you want
    // your interrupt service routine to permit other maskable interrupts, you must clear
    // the I flag in your code.

    // The Decimal flag controls how the 6502 adds and subtracts. If set, arithmetic
    // is carried out in packed binary coded decimal. This flag is unchanged by interrupts
    // and is unknown on power-up. The implication is that a CLD should be included in boot
    // or interrupt coding.

    // The Overflow flag is generally misunderstood and therefore under-utilised.
    // After an ADC or SBC instruction, the overflow flag will be set if the twos complement
    // result is less than -128 or greater than +127, and it will cleared otherwise. In twos
    // complement, $80 through $FF represents -128 through -1, and $00 through $7F represents
    // 0 through +127. Thus, after:

    static constexpr uint8_t kInstructionCycles = 2;

    void ExecuteTest() {
        auto [opcode, name, flag, state] = GetParam();
        expected_regs.SetFlag(flag, state);
        // expected_cycles = kInstructionCycles;
        expected_code_length = 1;
        RegisterBaseTest::Execute(MakeCode(opcode));
    }
};

TEST_P(FlagChangeTest, ) {
    ExecuteTest();
}

std::vector<FlagChangeTestArg> GetFlagChangeTestCases() {
    using Flags = Registers::Flags;
    return {
        {INS_CLC, "CLC", Flags::Carry, false},       //
        {INS_SEC, "SEC", Flags::Carry, true},        //
        {INS_CLD, "CLD", Flags::DecimalMode, false}, //
        {INS_SED, "SED", Flags::DecimalMode, true},  //
        {INS_CLI, "CLI", Flags::IRQB, false},        //
        {INS_SEI, "SEI", Flags::IRQB, true},         //
        {INS_CLV, "CLV", Flags::Overflow, false},    //
    };
}

INSTANTIATE_TEST_SUITE_P(, FlagChangeTest, ::testing::ValuesIn(GetFlagChangeTestCases()),
                         [](const auto &info) -> std::string { return fmt::format("{}", std::get<1>(info.param)); });

class MiscTest : public RegisterBaseTest {};

TEST_F(MiscTest, NOP) {
    expected_code_length = 1;
    expected_cycles = 2;
    Execute(MakeCode(INS_NOP));
}

TEST_F(MiscTest, BRK) {
    // BRK (BReaK)
    // Affects Flags: B

    // MODE           SYNTAX       HEX LEN TIM
    // Implied       BRK           $00  1   7

    // BRK causes a non-maskable interrupt and increments the program counter by one.
    // Therefore an RTI will go to the address of the BRK +2 so that BRK may be used
    // to replace a two-byte instruction for debugging and the subsequent RTI will be correct.

    expected_regs.SetFlag(Flags::Brk, true);
    expected_code_length = 2;
    expected_cycles = 7;

    Execute(MakeCode(INS_BRK, (uint8_t)0));
}

} // namespace emu::cpu6502