#include "base_test.hpp"
#include <gtest/gtest.h>
#include <optional>

using namespace emu::cpu::opcode;

class RegisterBaseTest : public BaseTest {
public:
    void SetUp() override {
        BaseTest::SetUp();

        expected_regs.a = RandomByte();
        expected_regs.x = RandomByte();
        expected_regs.y = RandomByte();
        expected_regs.stack_pointer = RandomByte();
        cpu.reg = expected_regs;
    }

    void Execute(const std::vector<uint8_t> &data, uint64_t cycles) override { //
        BaseTest::Execute(data, cycles);
    }

    void TearDown() override { //
        BaseTest::TearDown();
    }
};

class RegIncDecTest : public RegisterBaseTest {
public:
    // Affect Flags: N Z
    // These instructions are implied mode, have a length of one byte and require two machine cycles.

    using Reg8 = emu::cpu::Reg8;
    using Reg8Ptr = Reg8(Registers::*);

    Reg8Ptr test_register;
    std::optional<uint8_t> result_byte;

    void Execute(const std::vector<uint8_t> &data, uint64_t cycles) override {
        expected_regs.SetFlag(Flags::Zero, result_byte.value() == 0);
        expected_regs.SetFlag(Flags::Negative, (result_byte.value() & 0x80) > 0);

        RegisterBaseTest::Execute(data, cycles);
    }
};

TEST_F(RegIncDecTest, INX) {
    test_register = &Registers::x;
    result_byte = expected_regs.x += 1;
    Execute(MakeCode(INS_INX), 1);
}

TEST_F(RegIncDecTest, DEX) {
    test_register = &Registers::x;
    result_byte = expected_regs.x -= 1;
    Execute(MakeCode(INS_DEX), 1);
}

TEST_F(RegIncDecTest, INY) {
    test_register = &Registers::y;
    result_byte = expected_regs.y += 1;
    Execute(MakeCode(INS_INY), 1);
}

TEST_F(RegIncDecTest, DEY) {
    test_register = &Registers::y;
    result_byte = expected_regs.y -= 1;
    Execute(MakeCode(INS_DEY), 1);
}

// TEST_F(RegIncDecTest, INC_ACC) {
//     test_register = &Registers::a;
//     result_byte = expected_regs.a += 1;
//     Execute(MakeCode(INS_INC_ACC), 1);
// }

// TEST_F(RegIncDecTest, DEC_ACC) {
//     test_register = &Registers::a;
//     result_byte = expected_regs.a -= 1;
//     Execute(MakeCode(INS_DEC_ACC), 1);
// }

class RegisterTransferTest : public RegisterBaseTest {
public:
    // Affect Flags: N Z
    // These instructions are implied mode, have a length of one byte and require two machine cycles.

    using Reg8 = emu::cpu::Reg8;
    using Reg8Ptr = Reg8(Registers::*);

    Reg8Ptr source_reg;
    Reg8Ptr target_reg;

    void Execute(const std::vector<uint8_t> &data, uint64_t cycles) override {
        auto source_byte = expected_regs.*source_reg;
        if (target_reg != &Registers::stack_pointer) {
            expected_regs.SetFlag(Flags::Zero, source_byte == 0);
            expected_regs.SetFlag(Flags::Negative, (source_byte & 0x80) > 0);
        }
        expected_regs.*target_reg = source_byte;
        RegisterBaseTest::Execute(data, cycles);
    }
};

TEST_F(RegisterTransferTest, TAX) {
    source_reg = &Registers::a;
    target_reg = &Registers::x;
    Execute(MakeCode(INS_TAX), 1);
}

TEST_F(RegisterTransferTest, TAY) {
    source_reg = &Registers::a;
    target_reg = &Registers::y;
    Execute(MakeCode(INS_TAY), 1);
}

TEST_F(RegisterTransferTest, TXA) {
    source_reg = &Registers::x;
    target_reg = &Registers::a;
    Execute(MakeCode(INS_TXA), 1);
}

TEST_F(RegisterTransferTest, TYA) {
    source_reg = &Registers::y;
    target_reg = &Registers::a;
    Execute(MakeCode(INS_TYA), 1);
}

TEST_F(RegisterTransferTest, TXS) {
    // MNEMONIC                        HEX TIM
    // TXS (Transfer X to Stack ptr)   $9A  2
    source_reg = &Registers::x;
    target_reg = &Registers::stack_pointer;
    Execute(MakeCode(INS_TXS), 2);
}
TEST_F(RegisterTransferTest, TSX) {
    // MNEMONIC                        HEX TIM
    // TSX (Transfer Stack ptr to X)   $BA  2
    source_reg = &Registers::stack_pointer;
    target_reg = &Registers::x;
    Execute(MakeCode(INS_TSX), 2);
}

using FlagChangeTestArg = std::tuple<Opcode, std::string, emu::cpu::Cpu6502::Registers::Flags, bool>;

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

    void Execute() {
        auto [opcode, name, flag, state] = GetParam();
        expected_regs.SetFlag(flag, state);
        RegisterBaseTest::Execute(MakeCode(opcode), kInstructionCycles);
    }
};

TEST_P(FlagChangeTest, ) {
    Execute();
}

std::vector<FlagChangeTestArg> GetFlagChangeTestCases() {
    using Flags = emu::cpu::Cpu6502::Registers::Flags;
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

INSTANTIATE_TEST_SUITE_P(FlagChange, FlagChangeTest, ::testing::ValuesIn(GetFlagChangeTestCases()),
                         [](const auto &info) -> std::string { return fmt::format("{}", std::get<1>(info.param)); });

class MiscTest : public RegisterBaseTest {};

TEST_F(MiscTest, NOP) {
    Execute(MakeCode(INS_NOP), 2);
}

TEST_F(MiscTest, BRK) {
    // BRK (BReaK)
    // Affects Flags: B

    // MODE           SYNTAX       HEX LEN TIM
    // Implied       BRK           $00  1   7

    // BRK causes a non-maskable interrupt and increments the program counter by one.
    // Therefore an RTI will go to the address of the BRK +2 so that BRK may be used
    // to replace a two-byte instruction for debugging and the subsequent RTI will be correct.

    Execute(MakeCode(INS_BRK, (uint8_t)0), 7);
}
