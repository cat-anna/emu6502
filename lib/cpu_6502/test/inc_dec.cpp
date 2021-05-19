#include "base_test.hpp"
#include <gtest/gtest.h>

namespace emu::cpu6502 {
namespace {

using namespace emu::cpu6502::opcode;

using OpFunctor = std::function<uint8_t(uint8_t)>;
using IncDecTestArg = std::tuple<Opcode, const char *, AddressMode, uint8_t, uint8_t, OpFunctor>;

class IncDecTest : public BaseTest, public ::testing::WithParamInterface<IncDecTestArg> {
public:
    std::optional<uint8_t> result_byte;
    std::optional<MemPtr> operation_address;

    // INC(INCrement memory)
    // DEC (DECrement memory)
    // Affects Flags: N Z

    void SetUp() override {
        random_reg_values = true;
        BaseTest::SetUp();

        auto [opcode, name, mode, len, cycles, func] = GetParam();
        expected_cycles = cycles;
        expected_code_length = len;
        result_byte = func(target_byte);

        expected_regs.SetFlag(Flags::Zero, result_byte.value() == 0);
        expected_regs.SetFlag(Flags::Negative, (result_byte.value() & 0x80) > 0);
    }

    void Execute(const std::vector<uint8_t> &data) override {
        BaseTest::Execute(data);
        VerifyMemory(target_address, {result_byte.value()});
    }
};

TEST_P(IncDecTest, ) {
    auto [opcode, name, mode, len, cycles, func] = GetParam();
    Execute(MakeCode(opcode, mode));
}

std::vector<IncDecTestArg> GetIncDecTestCases() {
    auto INC = [](uint8_t a) -> uint8_t { return a + 1; };
    auto DEC = [](uint8_t a) -> uint8_t { return a - 1; };
    return {
        // MODE           SYNTAX       HEX LEN TIM
        // Zero Page     INC $44       $E6  2   5
        {INS_INC_ZP, "INC", AddressMode::ZP, 2_u8, 5_u8, INC},
        // Zero Page,X   INC $44,X     $F6  2   6
        {INS_INC_ZPX, "INC", AddressMode::ZPX, 2_u8, 6_u8, INC},
        // Absolute      INC $4400     $EE  3   6
        {INS_INC_ABS, "INC", AddressMode::ABS, 3_u8, 6_u8, INC},
        // Absolute,X    INC $4400,X   $FE  3   7
        {INS_INC_ABSX, "INC", AddressMode::ABSX, 3_u8, 7_u8, INC},
        // Zero Page     DEC $44       $C6  2   5
        {INS_DEC_ZP, "DEC", AddressMode::ZP, 2_u8, 5_u8, DEC},
        // Zero Page,X   DEC $44,X     $D6  2   6
        {INS_DEC_ZPX, "DEC", AddressMode::ZPX, 2_u8, 6_u8, DEC},
        // Absolute      DEC $4400     $CE  3   6
        {INS_DEC_ABS, "DEC", AddressMode::ABS, 3_u8, 6_u8, DEC},
        // Absolute,X    DEC $4400,X   $DE  3   7
        {INS_DEC_ABSX, "DEC", AddressMode::ABSX, 3_u8, 7_u8, DEC},
    };
}

INSTANTIATE_TEST_SUITE_P(, IncDecTest, ::testing::ValuesIn(GetIncDecTestCases()), GenTestNameFunc());

} // namespace
} // namespace emu::cpu6502