#include "base_test.hpp"
#include <gtest/gtest.h>
#include <optional>

using namespace emu6502::cpu::opcode;

using CompareTestArg = std::tuple<Opcode, AddressMode, uint8_t, uint8_t, emu6502::cpu::Cpu6502::Reg8Ptr>;

class CompareTest : public BaseTest, public ::testing::WithParamInterface<CompareTestArg> {
public:
    // CMP (CoMPare accumulator)
    // Affects Flags: N Z C

    // Compare sets flags as if a subtraction had been carried out.
    // If the value in the accumulator is equal or greater than the compared value,
    // the Carry will be set. The equal (Z) and negative (N) flags will be set based
    // on equality or lack thereof and the sign (i.e. A>=$80) of the accumulator.

    void SetUp() override { //
        BaseTest::SetUp();

        expected_regs.a = RandomByte();
        expected_regs.x = RandomByte();
        expected_regs.y = RandomByte();
        expected_regs.stack_pointer = RandomByte();

        cpu.reg = expected_regs;
    }

    void Execute(const std::vector<uint8_t> &data, uint64_t cycles) override { //
        auto [opcode, mode, len, _cycles, test_register] = GetParam();

        auto v = expected_regs.*test_register;

        expected_regs.SetFlag(Flags::Zero, v == target_byte);
        expected_regs.SetFlag(Flags::Carry, v >= target_byte);
        auto result = v - target_byte;
        expected_regs.SetFlag(Flags::Negative, (result & 0x80) > 0);

        std::cout << fmt::format("TEST R={:02x} OPERAND={:02x} F={}\n", v, target_byte, expected_regs.DumpFlags());
        BaseTest::Execute(data, cycles);
    }

    void TearDown() override { //
        BaseTest::TearDown();
    }
};

TEST_P(CompareTest, ) {
    auto [opcode, mode, len, cycles, reg] = GetParam();
    Execute(MakeCode(opcode, mode), cycles);
}

std::vector<CompareTestArg> GetCMPTestCases() {
    using Registers = emu6502::cpu::Cpu6502::Registers;
    // + add 1 cycle if page boundary crossed
    return {
        // MODE           SYNTAX       HEX LEN TIM
        // Immediate     CMP #$44      $C9  2   2
        {INS_CMP, AddressMode::IM, 2, 2, &Registers::a},

        // Zero Page     CMP $44       $C5  2   3
        {INS_CMP_ZP, AddressMode::ZP, 2, 3, &Registers::a},

        // Zero Page,X   CMP $44,X     $D5  2   4
        {INS_CMP_ZPX, AddressMode::ZPX, 2, 4, &Registers::a},

        // Absolute      CMP $4400     $CD  3   4
        {INS_CMP_ABS, AddressMode::ABS, 3, 4, &Registers::a},

        // Absolute,X    CMP $4400,X   $DD  3   4+
        {INS_CMP_ABSX, AddressMode::ABSX, 3, 4, &Registers::a},

        // Absolute,Y    CMP $4400,Y   $D9  3   4+
        {INS_CMP_ABSY, AddressMode::ABSY, 3, 4, &Registers::a},

        // Indirect,X    CMP ($44,X)   $C1  2   6
        {INS_CMP_INDX, AddressMode::INDX, 2, 6, &Registers::a},

        // Indirect,Y    CMP ($44),Y   $D1  2   5+
        {INS_CMP_INDY, AddressMode::INDY, 2, 5, &Registers::a},
    };
}

std::vector<CompareTestArg> GetCPXTestCases() {
    using Registers = emu6502::cpu::Cpu6502::Registers;
    return {
        // MODE           SYNTAX       HEX LEN TIM
        // Immediate     CPX #$44      $E0  2   2
        {INS_CPX, AddressMode::IM, 2, 2, &Registers::x},

        // Zero Page     CPX $44       $E4  2   3
        {INS_CPX_ZP, AddressMode::ZP, 2, 3, &Registers::x},

        // Absolute      CPX $4400     $EC  3   4
        {INS_CPX_ABS, AddressMode::ABS, 3, 4, &Registers::x},
    };
}

std::vector<CompareTestArg> GetCPYTestCases() {
    using Registers = emu6502::cpu::Cpu6502::Registers;
    return {
        // MODE           SYNTAX       HEX LEN TIM
        // Immediate     CPY #$44      $C0  2   2
        {INS_CPY, AddressMode::IM, 2, 2, &Registers::y},

        // Zero Page     CPY $44       $C4  2   3
        {INS_CPY_ZP, AddressMode::ZP, 2, 3, &Registers::y},

        // Absolute      CPY $4400     $CC  3   4
        {INS_CPY_ABS, AddressMode::ABS, 3, 4, &Registers::y},
    };
}

INSTANTIATE_TEST_SUITE_P(CMP, CompareTest, ::testing::ValuesIn(GetCMPTestCases()), GenTestNameFunc("CMP"));
INSTANTIATE_TEST_SUITE_P(CPX, CompareTest, ::testing::ValuesIn(GetCPXTestCases()), GenTestNameFunc("CPX"));
INSTANTIATE_TEST_SUITE_P(CPY, CompareTest, ::testing::ValuesIn(GetCPYTestCases()), GenTestNameFunc("CPY"));
