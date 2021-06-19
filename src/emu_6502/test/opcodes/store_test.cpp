#include "base_test.hpp"
#include <gtest/gtest.h>
#include <optional>

namespace emu::emu6502::test {
namespace {

using StoreTestArg = std::tuple<Opcode, const char *, AddressMode, uint8_t, uint8_t, Reg8Ptr>;
class StoreTest : public BaseTest, public ::testing::WithParamInterface<StoreTestArg> {
public:
    void SetUp() override {
        BaseTest::SetUp();
        auto [opcode, name, mode, len, cycles, reg] = GetParam();

        expected_code_length = len;
        expected_cycles = cycles;

        (cpu.reg.*reg) = (expected_regs.*reg) = target_byte;
    }

    void TearDown() override {
        BaseTest::TearDown();
        VerifyMemory(target_address, {target_byte});
    }
};

TEST_P(StoreTest, ) {
    auto [opcode, name, mode, len, cycles, reg] = GetParam();
    Execute(MakeCode(opcode, mode));
}

std::vector<StoreTestArg> GetStoreTestCases() {
    // + add 1 cycle if page boundary crossed
    return {
        // MODE           SYNTAX       HEX LEN TIM
        // Absolute      STA $4400     $8D  3   4
        {INS_STA_ABS, "STA", AddressMode::ABS, 3_u8, 4_u8, &Registers::a},
        // Zero Page     STA $44       $85  2   3
        {INS_STA_ZP, "STA", AddressMode::ZP, 2_u8, 3_u8, &Registers::a},
        // Zero Page,X   STA $44,X     $95  2   4
        {INS_STA_ZPX, "STA", AddressMode::ZPX, 2_u8, 4_u8, &Registers::a},
        // Absolute,X    STA $4400,X   $9D  3   5
        {INS_STA_ABSX, "STA", AddressMode::ABSX, 3_u8, 5_u8, &Registers::a},
        // Absolute,Y    STA $4400,Y   $99  3   5
        {INS_STA_ABSY, "STA", AddressMode::ABSY, 3_u8, 5_u8, &Registers::a},
        // Indirect,X    STA ($44,X)   $81  2   6
        {INS_STA_INDX, "STA", AddressMode::INDX, 2_u8, 6_u8, &Registers::a},
        // Indirect,Y    STA ($44),Y   $91  2   6
        {INS_STA_INDY, "STA", AddressMode::INDY, 2_u8, 6_u8, &Registers::a},
        // Zero Page     STX $44       $86  2   3
        {INS_STX_ZP, "STX", AddressMode::ZP, 2_u8, 3_u8, &Registers::x},
        // Zero Page,Y   STX $44,Y     $96  2   4
        {INS_STX_ZPY, "STX", AddressMode::ZPY, 2_u8, 4_u8, &Registers::x},
        // Absolute      STX $4400     $8E  3   4
        {INS_STX_ABS, "STX", AddressMode::ABS, 3_u8, 4_u8, &Registers::x},
        // Zero Page     STY $44       $84  2   3
        {INS_STY_ZP, "STY", AddressMode::ZP, 2_u8, 3_u8, &Registers::y},
        // Zero Page,X   STY $44,X     $94  2   4
        {INS_STY_ZPX, "STY", AddressMode::ZPX, 2_u8, 4_u8, &Registers::y},
        // Absolute      STY $4400     $8C  3   4
        {INS_STY_ABS, "STY", AddressMode::ABS, 3_u8, 4_u8, &Registers::y},

        // {INS_STY_ABS, "STA", AddressMode::INDZP, ?_u8, ?_u8, &Registers::a},
    };
}

INSTANTIATE_TEST_SUITE_P(, StoreTest, ::testing::ValuesIn(GetStoreTestCases()), GenTestNameFunc());

} // namespace
} // namespace emu::emu6502::test