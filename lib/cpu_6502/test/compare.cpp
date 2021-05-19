#include "base_test.hpp"
#include <gtest/gtest.h>
#include <optional>

namespace emu::cpu6502 {
namespace {

using namespace emu::cpu6502::opcode;

using CompareTestArg = std::tuple<Opcode, const char *, AddressMode, uint8_t, uint8_t, emu::cpu6502::Reg8Ptr>;

class CompareTest : public BaseTest, public ::testing::WithParamInterface<CompareTestArg> {
public:
    // CMP (CoMPare accumulator)
    // Affects Flags: N Z C

    // Compare sets flags as if a subtraction had been carried out.
    // If the value in the accumulator is equal or greater than the compared value,
    // the Carry will be set. The equal (Z) and negative (N) flags will be set based
    // on equality or lack thereof and the sign (i.e. A>=$80) of the accumulator.

    void SetUp() override {
        random_reg_values = true;
        BaseTest::SetUp();
    }

    void Execute(const std::vector<uint8_t> &data) override {
        auto [opcode, name, mode, len, cycles, test_register] = GetParam();
        expected_cycles = cycles;
        expected_code_length = len;

        auto v = expected_regs.*test_register;

        expected_regs.SetFlag(Flags::Zero, v == target_byte);
        expected_regs.SetFlag(Flags::Carry, v >= target_byte);
        auto result = v - target_byte;
        expected_regs.SetFlag(Flags::Negative, (result & 0x80) > 0);

        std::cout << fmt::format("TEST R={:02x} OPERAND={:02x} F={}\n", v, target_byte, expected_regs.DumpFlags());
        BaseTest::Execute(data);
    }
};

TEST_P(CompareTest, ) {
    auto [opcode, name, mode, len, cycles, reg] = GetParam();
    Execute(MakeCode(opcode, mode));
}

std::vector<CompareTestArg> GetTestCases() {
    // + add 1 cycle if page boundary crossed
    return {
        // MODE           SYNTAX       HEX LEN TIM
        // Immediate     CMP #$44      $C9  2   2
        {INS_CMP, "CMP", AddressMode::IM, 2_u8, 2_u8, &Registers::a},
        // Zero Page     CMP $44       $C5  2   3
        {INS_CMP_ZP, "CMP", AddressMode::ZP, 2_u8, 3_u8, &Registers::a},
        // Zero Page,X   CMP $44,X     $D5  2   4
        {INS_CMP_ZPX, "CMP", AddressMode::ZPX, 2_u8, 4_u8, &Registers::a},
        // Absolute      CMP $4400     $CD  3   4
        {INS_CMP_ABS, "CMP", AddressMode::ABS, 3_u8, 4_u8, &Registers::a},
        // Absolute,X    CMP $4400,X   $DD  3   4+
        {INS_CMP_ABSX, "CMP", AddressMode::ABSX, 3_u8, 4_u8, &Registers::a},
        // Absolute,Y    CMP $4400,Y   $D9  3   4+
        {INS_CMP_ABSY, "CMP", AddressMode::ABSY, 3_u8, 4_u8, &Registers::a},
        // Indirect,X    CMP ($44,X)   $C1  2   6
        {INS_CMP_INDX, "CMP", AddressMode::INDX, 2_u8, 6_u8, &Registers::a},
        // Indirect,Y    CMP ($44),Y   $D1  2   5+
        {INS_CMP_INDY, "CMP", AddressMode::INDY, 2_u8, 5_u8, &Registers::a},
        // Immediate     CPX #$44      $E0  2   2
        {INS_CPX, "CPX", AddressMode::IM, 2_u8, 2_u8, &Registers::x},
        // Zero Page     CPX $44       $E4  2   3
        {INS_CPX_ZP, "CPX", AddressMode::ZP, 2_u8, 3_u8, &Registers::x},
        // Absolute      CPX $4400     $EC  3   4
        {INS_CPX_ABS, "CPX", AddressMode::ABS, 3_u8, 4_u8, &Registers::x},
        // Immediate     CPY #$44      $C0  2   2
        {INS_CPY, "CPY", AddressMode::IM, 2_u8, 2_u8, &Registers::y},
        // Zero Page     CPY $44       $C4  2   3
        {INS_CPY_ZP, "CPY", AddressMode::ZP, 2_u8, 3_u8, &Registers::y},
        // Absolute      CPY $4400     $CC  3   4
        {INS_CPY_ABS, "CPY", AddressMode::ABS, 3_u8, 4_u8, &Registers::y},
    };
}

INSTANTIATE_TEST_SUITE_P(, CompareTest, ::testing::ValuesIn(GetTestCases()), GenTestNameFunc());

} // namespace
} // namespace emu::cpu6502