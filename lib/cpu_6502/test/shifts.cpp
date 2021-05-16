#include "base_test.hpp"
#include <functional>
#include <gtest/gtest.h>
#include <optional>

namespace emu::cpu6502 {

using namespace emu::cpu6502::opcode;
using Registers = emu::cpu6502::Registers;

using OpFunctor = std::function<std::tuple<uint8_t, bool>(uint8_t, bool)>;
using LogicalTestArg = std::tuple<Opcode, const char *, AddressMode, uint8_t, uint8_t, OpFunctor>;

class ShiftTest : public BaseTest, public ::testing::WithParamInterface<LogicalTestArg> {
public:
    // ASL (Arithmetic Shift Left)
    // LSR (Logical Shift Right)
    // ROL (ROtate Left)
    // ROR (ROtate Right)
    // Affects Flags: N Z C

    void SetUp() override { //
        BaseTest::SetUp();

        auto [opcode, name, mode, len, cycles, func] = GetParam();

        expected_regs.a = mode == AddressMode::ACC ? target_byte : RandomByte();
        expected_regs.x = RandomByte();
        expected_regs.y = RandomByte();
        cpu.reg = expected_regs;
    }

    void Execute(bool carry) {
        cpu.reg.SetFlag(Flags::Carry, carry);

        auto [opcode, name, mode, len, cycles, func] = GetParam();
        auto [result, new_carry] = func(target_byte, carry);

        expected_regs.SetFlag(Flags::Carry, new_carry);
        expected_regs.SetFlag(Flags::Zero, result == 0);
        expected_regs.SetFlag(Flags::Negative, (result & 0x80) > 0);
        if (mode == AddressMode::ACC) {
            expected_regs.a = result;
        }

        BaseTest::Execute(MakeCode(opcode, mode), cycles);
        if (mode != AddressMode::ACC) {
            VerifyMemory(target_address, {result});
        }
    }
};

TEST_P(ShiftTest, WithCarry) {
    Execute(true);
}

TEST_P(ShiftTest, WithoutCarry) {
    Execute(false);
}

std::vector<LogicalTestArg> GetTestCases() {
    // ROR shifts all bits right one position. The Carry is shifted into bit 7 and the
    // original bit 0 is shifted into the Carry.
    auto ROR = [](uint8_t v, bool carry) -> std::tuple<uint8_t, bool> {
        uint8_t new_v = (v >> 1) | (carry ? 0x80 : 0);
        return std::make_tuple(new_v, (v & 0x01) != 0);
    };
    // ROL shifts all bits left one position. The Carry is shifted into bit 0 and the
    // original bit 7 is shifted into the Carry.
    auto ROL = [](uint8_t v, bool carry) -> std::tuple<uint8_t, bool> {
        uint8_t new_v = (v << 1) | (carry ? 0x01 : 0);
        return std::make_tuple(new_v, (v & 0x80) != 0);
    };
    // LSR shifts all bits right one position. 0 is shifted into bit 7 and the
    // original bit 0 is shifted into the Carry.
    auto LSR = [](uint8_t v, bool carry) -> std::tuple<uint8_t, bool> {
        uint8_t new_v = (v >> 1);
        return std::make_tuple(new_v, (v & 0x01) != 0);
    };
    // ASL shifts all bits left one position. 0 is shifted into bit 0 and the
    // original bit 7 is shifted into the Carry.
    auto ASL = [](uint8_t v, bool carry) -> std::tuple<uint8_t, bool> {
        uint8_t new_v = (v << 1);
        return std::make_tuple(new_v, (v & 0x80) != 0);
    };
    return {
        // MODE           SYNTAX       HEX LEN TIM
        // Accumulator   ROL A         $2A  1   2
        {INS_ROL, "ROL", AddressMode::ACC, 1_u8, 2_u8, ROL},
        // Zero Page     ROL $44       $26  2   5
        {INS_ROL_ZP, "ROL", AddressMode::ZP, 2_u8, 5_u8, ROL},
        // Zero Page,X   ROL $44,X     $36  2   6
        {INS_ROL_ZPX, "ROL", AddressMode::ZPX, 2_u8, 6_u8, ROL},
        // Absolute      ROL $4400     $2E  3   6
        {INS_ROL_ABS, "ROL", AddressMode::ABS, 3_u8, 6_u8, ROL},
        // Absolute,X    ROL $4400,X   $3E  3   7
        {INS_ROL_ABSX, "ROL", AddressMode::ABSX, 3_u8, 7_u8, ROL},
        // Accumulator   ROR A         $6A  1   2
        {INS_ROR, "ROR", AddressMode::ACC, 1_u8, 2_u8, ROR},
        // Zero Page     ROR $44       $66  2   5
        {INS_ROR_ZP, "ROR", AddressMode::ZP, 2_u8, 5_u8, ROR},
        // Zero Page,X   ROR $44,X     $76  2   6
        {INS_ROR_ZPX, "ROR", AddressMode::ZPX, 2_u8, 6_u8, ROR},
        // Absolute      ROR $4400     $6E  3   6
        {INS_ROR_ABS, "ROR", AddressMode::ABS, 3_u8, 6_u8, ROR},
        // Absolute,X    ROR $4400,X   $7E  3   7
        {INS_ROR_ABSX, "ROR", AddressMode::ABSX, 3_u8, 7_u8, ROR},
        // Accumulator   ASL A         $0A  1   2
        {INS_ASL, "ASL", AddressMode::ACC, 1_u8, 2_u8, ASL},
        // Zero Page     ASL $44       $06  2   5
        {INS_ASL_ZP, "ASL", AddressMode::ZP, 2_u8, 5_u8, ASL},
        // Zero Page,X   ASL $44,X     $16  2   6
        {INS_ASL_ZPX, "ASL", AddressMode::ZPX, 2_u8, 6_u8, ASL},
        // Absolute      ASL $4400     $0E  3   6
        {INS_ASL_ABS, "ASL", AddressMode::ABS, 3_u8, 6_u8, ASL},
        // Absolute,X    ASL $4400,X   $1E  3   7
        {INS_ASL_ABSX, "ASL", AddressMode::ABSX, 3_u8, 7_u8, ASL},
        // Accumulator   LSR A         $4A  1   2
        {INS_LSR, "LSR", AddressMode::ACC, 1_u8, 2_u8, LSR},
        // Zero Page     LSR $44       $46  2   5
        {INS_LSR_ZP, "LSR", AddressMode::ZP, 2_u8, 5_u8, LSR},
        // Zero Page,X   LSR $44,X     $56  2   6
        {INS_LSR_ZPX, "LSR", AddressMode::ZPX, 2_u8, 6_u8, LSR},
        // Absolute      LSR $4400     $4E  3   6
        {INS_LSR_ABS, "LSR", AddressMode::ABS, 3_u8, 6_u8, LSR},
        // Absolute,X    LSR $4400,X   $5E  3   7
        {INS_LSR_ABSX, "LSR", AddressMode::ABSX, 3_u8, 7_u8, LSR},
    };
}

INSTANTIATE_TEST_SUITE_P(, ShiftTest, ::testing::ValuesIn(GetTestCases()), [](const auto &info) {
    return fmt::format("{}_{}", std::get<1>(info.param), to_string(std::get<2>(info.param)));
});

} // namespace emu::cpu6502