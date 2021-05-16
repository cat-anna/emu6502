#include "base_test.hpp"
#include <functional>
#include <gtest/gtest.h>
#include <optional>

using namespace emu::cpu::opcode;

using OpFunctor = std::function<uint8_t(uint8_t, uint8_t)>;
using LogicalTestArg = std::tuple<Opcode, AddressMode, uint8_t, uint8_t, OpFunctor>;

class LogicalTest : public BaseTest, public ::testing::WithParamInterface<LogicalTestArg> {
public:
    // Affects Flags: N Z
    // AND (bitwise AND with accumulator)
    // EOR (bitwise Exclusive OR)
    // ORA (bitwise OR with Accumulator)

    void SetUp() override { //
        BaseTest::SetUp();

        expected_regs.a = RandomByte();
        expected_regs.x = RandomByte();
        expected_regs.y = RandomByte();
        expected_regs.stack_pointer = RandomByte();
        cpu.reg = expected_regs;
    }

    void Execute(const std::vector<uint8_t> &data) {
        auto [opcode, mode, len, cycles, func] = GetParam();
        uint8_t result = func(expected_regs.a, target_byte);

        expected_regs.SetFlag(Flags::Zero, result == 0);
        expected_regs.SetFlag(Flags::Negative, (result & 0x80) > 0);
        expected_regs.a = result;

        BaseTest::Execute(data, cycles);
    }
};

TEST_P(LogicalTest, ) {
    auto [opcode, mode, len, cycles, func] = GetParam();
    Execute(MakeCode(opcode, mode));
}

std::vector<LogicalTestArg> GetANDTestCases() {
    // TODO: + add 1 cycle if page boundary crossed
    auto AND = [](uint8_t a, uint8_t b) { return a & b; };
    return {
        // MODE           SYNTAX       HEX LEN TIM
        // Immediate     AND #$44      $29  2   2
        {INS_AND_IM, AddressMode::IM, 2, 2, AND},
        // Zero Page     AND $44       $25  2   3
        {INS_AND_ZP, AddressMode::ZP, 2, 3, AND},
        // Zero Page,X   AND $44,X     $35  2   4
        {INS_AND_ZPX, AddressMode::ZPX, 2, 4, AND},
        // Absolute      AND $4400     $2D  3   4
        {INS_AND_ABS, AddressMode::ABS, 3, 4, AND},
        // Absolute,X    AND $4400,X   $3D  3   4+
        {INS_AND_ABSX, AddressMode::ABSX, 3, 4, AND},
        // Absolute,Y    AND $4400,Y   $39  3   4+
        {INS_AND_ABSY, AddressMode::ABSY, 3, 4, AND},
        // Indirect,X    AND ($44,X)   $21  2   6
        {INS_AND_INDX, AddressMode::INDX, 2, 6, AND},
        // Indirect,Y    AND ($44),Y   $31  2   5+
        {INS_AND_INDY, AddressMode::INDY, 2, 5, AND},
    };
}

std::vector<LogicalTestArg> GetEORTestCases() {
    // TODO: + add 1 cycle if page boundary crossed
    auto EOR = [](uint8_t a, uint8_t b) { return a ^ b; };
    return {
        // MODE           SYNTAX       HEX LEN TIM
        // Immediate     EOR #$44      $49  2   2
        {INS_EOR_IM, AddressMode::IM, 2, 2, EOR},
        // Zero Page     EOR $44       $45  2   3
        {INS_EOR_ZP, AddressMode::ZP, 2, 3, EOR},
        // Zero Page,X   EOR $44,X     $55  2   4
        {INS_EOR_ZPX, AddressMode::ZPX, 2, 4, EOR},
        // Absolute      EOR $4400     $4D  3   4
        {INS_EOR_ABS, AddressMode::ABS, 3, 4, EOR},
        // Absolute,X    EOR $4400,X   $5D  3   4+
        {INS_EOR_ABSX, AddressMode::ABSX, 3, 4, EOR},
        // Absolute,Y    EOR $4400,Y   $59  3   4+
        {INS_EOR_ABSY, AddressMode::ABSY, 3, 4, EOR},
        // Indirect,X    EOR ($44,X)   $41  2   6
        {INS_EOR_INDX, AddressMode::INDX, 2, 6, EOR},
        // Indirect,Y    EOR ($44),Y   $51  2   5+
        {INS_EOR_INDY, AddressMode::INDY, 2, 5, EOR},
    };
}

std::vector<LogicalTestArg> GetORATestCases() {
    // TODO: + add 1 cycle if page boundary crossed
    auto ORA = [](uint8_t a, uint8_t b) { return a | b; };
    return {
        // MODE           SYNTAX       HEX LEN TIM
        // Immediate     ORA #$44      $09  2   2
        {INS_ORA_IM, AddressMode::IM, 2, 2, ORA},
        // Zero Page     ORA $44       $05  2   3
        {INS_ORA_ZP, AddressMode::ZP, 2, 3, ORA},
        // Zero Page,X   ORA $44,X     $15  2   4
        {INS_ORA_ZPX, AddressMode::ZPX, 2, 4, ORA},
        // Absolute      ORA $4400     $0D  3   4
        {INS_ORA_ABS, AddressMode::ABS, 3, 4, ORA},
        // Absolute,X    ORA $4400,X   $1D  3   4+
        {INS_ORA_ABSX, AddressMode::ABSX, 3, 4, ORA},
        // Absolute,Y    ORA $4400,Y   $19  3   4+
        {INS_ORA_ABSY, AddressMode::ABSY, 3, 4, ORA},
        // Indirect,X    ORA ($44,X)   $01  2   6
        {INS_ORA_INDX, AddressMode::INDX, 2, 6, ORA},
        // Indirect,Y    ORA ($44),Y   $11  2   5+
        {INS_ORA_INDY, AddressMode::INDY, 2, 5, ORA},
    };
}

INSTANTIATE_TEST_SUITE_P(AND, LogicalTest, ::testing::ValuesIn(GetANDTestCases()), GenTestNameFunc("AND"));
INSTANTIATE_TEST_SUITE_P(EOR, LogicalTest, ::testing::ValuesIn(GetEORTestCases()), GenTestNameFunc("EOR"));
INSTANTIATE_TEST_SUITE_P(ORA, LogicalTest, ::testing::ValuesIn(GetORATestCases()), GenTestNameFunc("ORA"));

class BitTest : public BaseTest, public ::testing::WithParamInterface<LogicalTestArg> {
public:
    // BIT (test BITs)
    // Affects Flags: N V Z

    // BIT sets the Z flag as though the value in the address tested were ANDed with the accumulator.
    // The N and V flags are set to match bits 7 and 6 respectively in the value stored at the tested address.
    // BIT is often used to skip one or two following bytes as in:

    void SetUp() override { //
        BaseTest::SetUp();

        expected_regs.a = RandomByte();
        expected_regs.x = RandomByte();
        expected_regs.y = RandomByte();
        expected_regs.stack_pointer = RandomByte();
        cpu.reg = expected_regs;
    }

    void Execute(const std::vector<uint8_t> &data) {
        auto [opcode, mode, len, cycles, func] = GetParam();
        uint8_t result = func(expected_regs.a, target_byte);

        expected_regs.SetFlag(Flags::Zero, result == 0);
        expected_regs.SetFlag(Flags::Negative, (result & 0x80) > 0);
        expected_regs.SetFlag(Flags::Overflow, (result & 0x40) > 0);

        BaseTest::Execute(data, cycles);
    }
};

TEST_P(BitTest, ) {
    auto [opcode, mode, len, cycles, func] = GetParam();
    Execute(MakeCode(opcode, mode));
}

std::vector<LogicalTestArg> GetBITTestCases() {
    // TODO: + add 1 cycle if page boundary crossed
    auto AND = [](uint8_t a, uint8_t b) { return a & b; };
    return {
        // MODE           SYNTAX       HEX LEN TIM
        // Zero Page     BIT $44       $24  2   3
        {INS_BIT_ZP, AddressMode::ZP, 2, 3, AND},
        // Absolute      BIT $4400     $2C  3   4
        {INS_BIT_ABS, AddressMode::ABS, 3, 4, AND},
    };
}

INSTANTIATE_TEST_SUITE_P(, BitTest, ::testing::ValuesIn(GetBITTestCases()), GenTestNameFunc("BIT"));
