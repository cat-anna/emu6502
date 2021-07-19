#include "base_test.hpp"
#include <functional>
#include <gtest/gtest.h>
#include <optional>

namespace emu::emu6502::test {
namespace {

using OpFunctor = std::function<uint8_t(uint8_t, uint8_t)>;
using LogicalTestArg =
    std::tuple<Opcode, const char *, AddressMode, uint8_t, uint8_t, OpFunctor>;

class LogicalTest : public BaseTest,
                    public ::testing::WithParamInterface<LogicalTestArg> {
public:
    // Affects Flags: N Z
    // AND (bitwise AND with accumulator)
    // EOR (bitwise Exclusive OR)
    // ORA (bitwise OR with Accumulator)

    void SetUp() override {
        random_reg_values = true;
        BaseTest::SetUp();
    }

    void Execute(const std::vector<uint8_t> &data) override {
        auto [opcode, name, mode, len, cycles, func] = GetParam();
        expected_code_length = len;
        expected_cycles = cycles;

        uint8_t result = func(expected_regs.a, target_byte);

        expected_regs.SetFlag(Flags::Zero, result == 0);
        expected_regs.SetFlag(Flags::Negative, (result & 0x80) > 0);
        expected_regs.a = result;

        BaseTest::Execute(data);
    }
};

TEST_P(LogicalTest, ) {
    auto [opcode, name, mode, len, cycles, func] = GetParam();
    Execute(MakeCode(opcode, mode));
}

std::vector<LogicalTestArg> GetLogicalTestCases() {
    // TODO: + add 1 cycle if page boundary crossed
    auto AND = [](uint8_t a, uint8_t b) -> uint8_t { return a & b; };
    auto EOR = [](uint8_t a, uint8_t b) -> uint8_t { return a ^ b; };
    auto ORA = [](uint8_t a, uint8_t b) -> uint8_t { return a | b; };
    return {
        // MODE           SYNTAX       HEX LEN TIM
        // Immediate     AND #$44      $29  2   2
        {INS_AND_IM, "AND", AddressMode::IM, 2_u8, 2_u8, AND},
        // Zero Page     AND $44       $25  2   3
        {INS_AND_ZP, "AND", AddressMode::ZP, 2_u8, 3_u8, AND},
        // Zero Page,X   AND $44,X     $35  2   4
        {INS_AND_ZPX, "AND", AddressMode::ZPX, 2_u8, 4_u8, AND},
        // Absolute      AND $4400     $2D  3   4
        {INS_AND_ABS, "AND", AddressMode::ABS, 3_u8, 4_u8, AND},
        // Absolute,X    AND $4400,X   $3D  3   4+
        {INS_AND_ABSX, "AND", AddressMode::ABSX, 3_u8, 4_u8, AND},
        // Absolute,Y    AND $4400,Y   $39  3   4+
        {INS_AND_ABSY, "AND", AddressMode::ABSY, 3_u8, 4_u8, AND},
        // Indirect,X    AND ($44,X)   $21  2   6
        {INS_AND_INDX, "AND", AddressMode::INDX, 2_u8, 6_u8, AND},
        // Indirect,Y    AND ($44),Y   $31  2   5+
        {INS_AND_INDY, "AND", AddressMode::INDY, 2_u8, 5_u8, AND},
        // Immediate     EOR #$44      $49  2   2
        {INS_EOR_IM, "EOR", AddressMode::IM, 2_u8, 2_u8, EOR},
        // Zero Page     EOR $44       $45  2   3
        {INS_EOR_ZP, "EOR", AddressMode::ZP, 2_u8, 3_u8, EOR},
        // Zero Page,X   EOR $44,X     $55  2   4
        {INS_EOR_ZPX, "EOR", AddressMode::ZPX, 2_u8, 4_u8, EOR},
        // Absolute      EOR $4400     $4D  3   4
        {INS_EOR_ABS, "EOR", AddressMode::ABS, 3_u8, 4_u8, EOR},
        // Absolute,X    EOR $4400,X   $5D  3   4+
        {INS_EOR_ABSX, "EOR", AddressMode::ABSX, 3_u8, 4_u8, EOR},
        // Absolute,Y    EOR $4400,Y   $59  3   4+
        {INS_EOR_ABSY, "EOR", AddressMode::ABSY, 3_u8, 4_u8, EOR},
        // Indirect,X    EOR ($44,X)   $41  2   6
        {INS_EOR_INDX, "EOR", AddressMode::INDX, 2_u8, 6_u8, EOR},
        // Indirect,Y    EOR ($44),Y   $51  2   5+
        {INS_EOR_INDY, "EOR", AddressMode::INDY, 2_u8, 5_u8, EOR},
        // Immediate     ORA #$44      $09  2   2
        {INS_ORA_IM, "ORA", AddressMode::IM, 2_u8, 2_u8, ORA},
        // Zero Page     ORA $44       $05  2   3
        {INS_ORA_ZP, "ORA", AddressMode::ZP, 2_u8, 3_u8, ORA},
        // Zero Page,X   ORA $44,X     $15  2   4
        {INS_ORA_ZPX, "ORA", AddressMode::ZPX, 2_u8, 4_u8, ORA},
        // Absolute      ORA $4400     $0D  3   4
        {INS_ORA_ABS, "ORA", AddressMode::ABS, 3_u8, 4_u8, ORA},
        // Absolute,X    ORA $4400,X   $1D  3   4+
        {INS_ORA_ABSX, "ORA", AddressMode::ABSX, 3_u8, 4_u8, ORA},
        // Absolute,Y    ORA $4400,Y   $19  3   4+
        {INS_ORA_ABSY, "ORA", AddressMode::ABSY, 3_u8, 4_u8, ORA},
        // Indirect,X    ORA ($44,X)   $01  2   6
        {INS_ORA_INDX, "ORA", AddressMode::INDX, 2_u8, 6_u8, ORA},
        // Indirect,Y    ORA ($44),Y   $11  2   5+
        {INS_ORA_INDY, "ORA", AddressMode::INDY, 2_u8, 5_u8, ORA},
    };
}

INSTANTIATE_TEST_SUITE_P(, LogicalTest, ::testing::ValuesIn(GetLogicalTestCases()),
                         GenTestNameFunc());

class BitTest : public BaseTest, public ::testing::WithParamInterface<LogicalTestArg> {
public:
    // BIT (test BITs)
    // Affects Flags: N V Z

    // BIT sets the Z flag as though the value in the address tested were ANDed with the accumulator.
    // The N and V flags are set to match bits 7 and 6 respectively in the value stored at the tested address.
    // BIT is often used to skip one or two following bytes as in:

    void SetUp() override {
        random_reg_values = true;
        BaseTest::SetUp();
    }

    void Execute(const std::vector<uint8_t> &data) override {
        auto [opcode, name, mode, len, cycles, func] = GetParam();
        expected_code_length = len;
        expected_cycles = cycles;
        uint8_t result = func(expected_regs.a, target_byte);

        expected_regs.SetFlag(Flags::Zero, result == 0);
        expected_regs.SetFlag(Flags::Negative, (target_byte & 0x80) > 0);
        expected_regs.SetFlag(Flags::Overflow, (target_byte & 0x40) > 0);

        BaseTest::Execute(data);
    }
};

TEST_P(BitTest, ) {
    auto [opcode, name, mode, len, cycles, func] = GetParam();
    Execute(MakeCode(opcode, mode));
}

std::vector<LogicalTestArg> GetBITTestCases() {
    // TODO: + add 1 cycle if page boundary crossed
    auto AND = [](uint8_t a, uint8_t b) -> uint8_t { return a & b; };
    return {
        // MODE           SYNTAX       HEX LEN TIM
        // Zero Page     BIT $44       $24  2   3
        {INS_BIT_ZP, "BIT", AddressMode::ZP, 2_u8, 3_u8, AND},
        // Absolute      BIT $4400     $2C  3   4
        {INS_BIT_ABS, "BIT", AddressMode::ABS, 3_u8, 4_u8, AND},
    };
}

INSTANTIATE_TEST_SUITE_P(, BitTest, ::testing::ValuesIn(GetBITTestCases()),
                         GenTestNameFunc());

} // namespace
} // namespace emu::emu6502::test
