#include "base_test.hpp"
#include <gtest/gtest.h>
#include <optional>

namespace emu::emu6502::test {
namespace {

using LoadTestArg = std::tuple<Opcode, const char *, AddressMode, uint8_t, uint8_t, Reg8Ptr>;
class LoadTest : public BaseTest, public ::testing::WithParamInterface<LoadTestArg> {
public:
    // LDA(LoaD Accumulator)
    // LDX (LoaD X register)
    // LDY (LoaD Y register)
    // Affects Flags : N Z

    void SetUp() override {
        BaseTest::SetUp();
        auto [opcode, name, mode, len, cycles, reg] = GetParam();

        expected_code_length = len;
        expected_cycles = cycles;

        expected_regs.*reg = target_byte;
        expected_regs.SetFlag(Flags::Zero, target_byte == 0);
        expected_regs.SetFlag(Flags::Negative, (target_byte & 0x80) != 0);
    }
};

TEST_P(LoadTest, ) {
    auto [opcode, name, mode, len, cycles, reg] = GetParam();
    Execute(MakeCode(opcode, mode));
}

std::vector<LoadTestArg> GetLoadTestCases() {
    // + add 1 cycle if page boundary crossed
    return {
        // Absolute      LDA $4400     $AD  3   4
        {INS_LDA_ABS, "LDA", AddressMode::ABS, 3_u8, 4_u8, &Registers::a},
        // Immediate     LDA #$44      $A9  2   2
        {INS_LDA_IM, "LDA", AddressMode::IM, 2_u8, 2_u8, &Registers::a},
        // Zero Page     LDA $44       $A5  2   3
        {INS_LDA_ZP, "LDA", AddressMode::ZP, 2_u8, 3_u8, &Registers::a},
        // Zero Page,X   LDA $44,X     $B5  2   4
        {INS_LDA_ZPX, "LDA", AddressMode::ZPX, 2_u8, 4_u8, &Registers::a},
        // Absolute,X    LDA $4400,X   $BD  3   4+
        {INS_LDA_ABSX, "LDA", AddressMode::ABSX, 3_u8, 4_u8, &Registers::a},
        // Absolute,Y    LDA $4400,Y   $B9  3   4+
        {INS_LDA_ABSY, "LDA", AddressMode::ABSY, 3_u8, 4_u8, &Registers::a},
        // Indirect,X    LDA ($44,X)   $A1  2   6
        {INS_LDA_INDX, "LDA", AddressMode::INDX, 2_u8, 6_u8, &Registers::a},
        // Indirect,Y    LDA ($44),Y   $B1  2   5+
        {INS_LDA_INDY, "LDA", AddressMode::INDY, 2_u8, 5_u8, &Registers::a},
        // Immediate     LDX #$44      $A2  2   2
        {INS_LDX_IM, "LDX", AddressMode::IM, 2_u8, 2_u8, &Registers::x},
        // Zero Page     LDX $44       $A6  2   3
        {INS_LDX_ZP, "LDX", AddressMode::ZP, 2_u8, 3_u8, &Registers::x},
        // Zero Page,Y   LDX $44,Y     $B6  2   4
        {INS_LDX_ZPY, "LDX", AddressMode::ZPY, 2_u8, 4_u8, &Registers::x},
        // Absolute      LDX $4400     $AE  3   4
        {INS_LDX_ABS, "LDX", AddressMode::ABS, 3_u8, 4_u8, &Registers::x},
        // Absolute,Y    LDX $4400,Y   $BE  3   4+
        {INS_LDX_ABSY, "LDX", AddressMode::ABSY, 3_u8, 4_u8, &Registers::x},
        // Immediate     LDY #$44      $A0  2   2
        {INS_LDY_IM, "LDY", AddressMode::IM, 2_u8, 2_u8, &Registers::y},
        // Zero Page     LDY $44       $A4  2   3
        {INS_LDY_ZP, "LDY", AddressMode::ZP, 2_u8, 3_u8, &Registers::y},
        // Zero Page,X   LDY $44,X     $B4  2   4
        {INS_LDY_ZPX, "LDY", AddressMode::ZPX, 2_u8, 4_u8, &Registers::y},
        // Absolute      LDY $4400     $AC  3   4
        {INS_LDY_ABS, "LDY", AddressMode::ABS, 3_u8, 4_u8, &Registers::y},
        // Absolute,X    LDY $4400,X   $BC  3   4+
        {INS_LDY_ABSX, "LDY", AddressMode::ABSX, 3_u8, 4_u8, &Registers::y},
    };
}

INSTANTIATE_TEST_SUITE_P(, LoadTest, ::testing::ValuesIn(GetLoadTestCases()), GenTestNameFunc());

} // namespace
} // namespace emu::emu6502::test