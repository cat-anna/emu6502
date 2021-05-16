#include "base_test.hpp"
#include <gtest/gtest.h>
#include <optional>

namespace emu::cpu6502 {

using namespace emu::cpu6502::opcode;

using ArithmeticTestArg = std::tuple<Opcode, AddressMode, uint8_t, uint8_t>;

class ArithmeticBaseTest : public BaseTest {
public:
    std::optional<uint16_t> result;
    Registers initial_reg;

    void SetUp() override { //
        BaseTest::SetUp();

        expected_regs.a = RandomByte();
        expected_regs.x = RandomByte();
        expected_regs.y = RandomByte();
        expected_regs.stack_pointer = RandomByte();
        expected_regs.flags = cpu.reg.flags &= ~static_cast<uint8_t>(Flags::DecimalMode);

        cpu.reg = expected_regs;
        initial_reg = expected_regs;
    }

    void Execute(const std::vector<uint8_t> &data, uint64_t cycles) override { //
        const bool SignBitsAreEqual = ((initial_reg.a ^ target_byte) & 0x80) == 0;

        expected_regs.a = result.value() & 0xFF;

        expected_regs.SetFlag(Flags::Zero, expected_regs.a == 0);
        expected_regs.SetFlag(Flags::Carry, result.value() > 0xFF);
        expected_regs.SetFlag(Flags::Negative, (result.value() & 0x80) > 0);
        expected_regs.SetFlag(Flags::Overflow, SignBitsAreEqual && ((expected_regs.a ^ target_byte) & 0x80) != 0);

        BaseTest::Execute(data, cycles);

        std::cout << fmt::format("TEST OPERAND={:02x} C={:02x} A0={:02x} R={:04x} A1={:02x}\n", target_byte,
                                 initial_reg.CarryValue(), initial_reg.a, result.value(), cpu.reg.a);
    }

    void TearDown() override { //
        BaseTest::TearDown();
    }
};

class ADC : public ArithmeticBaseTest, public ::testing::WithParamInterface<ArithmeticTestArg> {
public:
    // ADC (ADd with Carry)
    // Affects Flags: N V Z C

    // ADC results are dependant on the setting of the decimal flag.
    // In decimal mode, addition is carried out on the assumption that the values
    // involved are packed BCD (Binary Coded Decimal).
    // There is no way to add without carry.

    void SetUp() override {
        ArithmeticBaseTest::SetUp();
        uint16_t r = cpu.reg.a;
        r += cpu.reg.CarryValue();
        r += target_byte;
        result = r;
    }
};

TEST_P(ADC, ) {
    auto [opcode, mode, len, cycles] = GetParam();
    Execute(MakeCode(opcode, mode), cycles);
}

std::vector<ArithmeticTestArg> GetADCTestCases() {
    // TODO: + add 1 cycle if page boundary crossed
    return {
        // MODE           SYNTAX       HEX LEN TIM
        // Immediate     ADC #$44      $69  2   2
        {INS_ADC, AddressMode::IM, 2_u8, 2_u8},

        // Absolute      ADC $4400     $6D  3   4
        {INS_ADC_ABS, AddressMode::ABS, 3_u8, 4_u8},

        // Zero Page     ADC $44       $65  2   3
        {INS_ADC_ZP, AddressMode::ZP, 2_u8, 3_u8},

        // Zero Page,X   ADC $44,X     $75  2   4
        {INS_ADC_ZPX, AddressMode::ZPX, 2_u8, 4_u8},

        // Absolute,X    ADC $4400,X   $7D  3   4+
        {INS_ADC_ABSX, AddressMode::ABSX, 3_u8, 4_u8},

        // Absolute,Y    ADC $4400,Y   $79  3   4+
        {INS_ADC_ABSY, AddressMode::ABSY, 3_u8, 4_u8},

        // Indirect,X    ADC ($44,X)   $61  2   6
        {INS_ADC_INDX, AddressMode::INDX, 2_u8, 6_u8},

        // Indirect,Y    ADC ($44),Y   $71  2   5+
        {INS_ADC_INDY, AddressMode::INDY, 2_u8, 5_u8},
    };
}

class SBC : public ArithmeticBaseTest, public ::testing::WithParamInterface<ArithmeticTestArg> {
public:
    // SBC (SuBtract with Carry)
    // Affects Flags: N V Z C

    // SBC results are dependant on the setting of the decimal flag.
    // In decimal mode, subtraction is carried out on the assumption that the values
    // involved are packed BCD (Binary Coded Decimal).
    // There is no way to subtract without the carry which works as an inverse borrow. i.e,
    // to subtract you set the carry before the operation. If the carry is cleared by the operation,
    // it indicates a borrow occurred.

    void SetUp() override {
        ArithmeticBaseTest::SetUp();
        uint16_t r = cpu.reg.a;
        r += cpu.reg.CarryValue();
        r -= target_byte;
        result = r;
    }
};

TEST_P(SBC, ) {
    auto [opcode, mode, len, cycles] = GetParam();
    Execute(MakeCode(opcode, mode), cycles);
}

std::vector<ArithmeticTestArg> GetSBCTestCases() {
    // TODO: + add 1 cycle if page boundary crossed
    return {
        // MODE           SYNTAX       HEX LEN TIM
        // Immediate     SBC #$44      $E9  2   2
        {INS_SBC, AddressMode::IM, 2_u8, 2_u8},

        // Absolute      SBC $4400     $ED  3   4
        {INS_SBC_ABS, AddressMode::ABS, 3_u8, 4_u8},

        // Zero Page     SBC $44       $E5  2   3
        {INS_SBC_ZP, AddressMode::ZP, 2_u8, 3_u8},

        // Zero Page,X   SBC $44,X     $F5  2   4
        {INS_SBC_ZPX, AddressMode::ZPX, 2_u8, 4_u8},

        // Absolute,X    SBC $4400,X   $FD  3   4+
        {INS_SBC_ABSX, AddressMode::ABSX, 3_u8, 4_u8},

        // Absolute,Y    SBC $4400,Y   $F9  3   4+
        {INS_SBC_ABSY, AddressMode::ABSY, 3_u8, 4_u8},

        // Indirect,X    SBC ($44,X)   $E1  2   6
        {INS_SBC_INDX, AddressMode::INDX, 2_u8, 6_u8},

        // Indirect,Y    SBC ($44),Y   $F1  2   5+
        {INS_SBC_INDY, AddressMode::INDY, 2_u8, 5_u8},
    };
}

INSTANTIATE_TEST_SUITE_P(ADC, ADC, ::testing::ValuesIn(GetADCTestCases()), GenTestNameFunc("ADC"));
INSTANTIATE_TEST_SUITE_P(SBC, SBC, ::testing::ValuesIn(GetSBCTestCases()), GenTestNameFunc("SBC"));

} // namespace emu::cpu6502