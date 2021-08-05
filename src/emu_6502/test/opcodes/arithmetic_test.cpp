#include "base_test.hpp"
#include <gtest/gtest.h>
#include <optional>

namespace emu::emu6502::test {
namespace {

using OpFunctor = std::function<std::tuple<bool, uint16_t>(uint8_t, uint8_t, uint8_t)>;
using ArithmeticTestArg =
    std::tuple<Opcode, std::string, AddressMode, uint8_t, uint8_t, OpFunctor, bool>;

class ArithmeticTest : public BaseTest,
                       public ::testing::WithParamInterface<ArithmeticTestArg> {
public:
    // SBC (SuBtract with Carry)
    // ADC (ADd with Carry)
    // Affects Flags: N V Z C

    std::optional<uint16_t> result;
    Registers initial_reg;
    bool subtract;

    void SetUp() override {
        auto [opcode, name, mode, len, cycles, op, crosspage] = GetParam();

        random_reg_values = true;
        SetupTestValues(crosspage);
        BaseTest::SetUp();

        expected_regs.flags = cpu.reg.flags &= ~static_cast<uint8_t>(Flags::DecimalMode);
        cpu.reg = expected_regs;
        initial_reg = expected_regs;

        auto [s, r] = op(cpu.reg.a, target_byte, cpu.reg.CarryValue());
        subtract = s;
        result = r;
        expected_cycles = cycles;
        expected_code_length = len;
    }

    void Execute(const std::vector<uint8_t> &data) override { //
        auto operand = subtract ? ~target_byte : target_byte;
        const bool SignBitsAreEqual = ((initial_reg.a ^ operand) & 0x80) == 0;

        expected_regs.a = result.value() & 0xFF;

        expected_regs.SetFlag(Flags::Zero, expected_regs.a == 0);
        expected_regs.SetFlag(Flags::Carry, subtract != (result.value() > 0xFF));
        expected_regs.SetFlag(Flags::Negative, (result.value() & 0x80) > 0);
        expected_regs.SetFlag(Flags::Overflow,
                              SignBitsAreEqual &&
                                  ((expected_regs.a ^ operand) & 0x80) != 0);

        BaseTest::Execute(data);

        std::cout << fmt::format(
            "TEST OPERAND={:02x} C={:02x} A0={:02x} R={:04x} A1={:02x}\n", target_byte,
            initial_reg.CarryValue(), initial_reg.a, result.value(), cpu.reg.a);
    }
};

TEST_P(ArithmeticTest, direct) {
    auto [opcode, name, mode, len, cycles, op, crosspage] = GetParam();
    Execute(MakeCode(opcode, mode));
}

std::vector<ArithmeticTestArg> GetTestCases() {
    // ADC results are dependant on the setting of the decimal flag.
    // In decimal mode, addition is carried out on the assumption that the values
    // involved are packed BCD (Binary Coded Decimal).
    // There is no way to add without carry.

    // SBC results are dependant on the setting of the decimal flag.
    // In decimal mode, subtraction is carried out on the assumption that the values
    // involved are packed BCD (Binary Coded Decimal).
    // There is no way to subtract without the carry which works as an inverse borrow. i.e,
    // to subtract you set the carry before the operation. If the carry is cleared by the operation,
    // it indicates a borrow occurred.

    auto ADD = [](uint8_t a, uint8_t b, uint8_t c) -> std::tuple<bool, uint16_t> {
        return {false, static_cast<uint16_t>(a + (b + c))};
    };
    auto SBC = [](uint8_t a, uint8_t b, uint8_t c) -> std::tuple<bool, uint16_t> {
        return {true, static_cast<uint16_t>(a - (b + (1 - c)))};
    };
    // + add 1 cycle if page boundary crossed
    return {
        // MODE           SYNTAX       HEX LEN TIM
        // Immediate     ADC #$44      $69  2   2
        {INS_ADC, "ADC", AddressMode::IM, 2_u8, 2_u8, ADD, false},
        // Absolute      ADC $4400     $6D  3   4
        {INS_ADC_ABS, "ADC", AddressMode::ABS, 3_u8, 4_u8, ADD, false},
        // Zero Page     ADC $44       $65  2   3
        {INS_ADC_ZP, "ADC", AddressMode::ZP, 2_u8, 3_u8, ADD, false},
        // Zero Page,X   ADC $44,X     $75  2   4
        {INS_ADC_ZPX, "ADC", AddressMode::ZPX, 2_u8, 4_u8, ADD, false},
        // Absolute,X    ADC $4400,X   $7D  3   4+
        {INS_ADC_ABSX, "ADC", AddressMode::ABSX, 3_u8, 4_u8, ADD, false},
        {INS_ADC_ABSX, "ADC_crosspage", AddressMode::ABSX, 3_u8, 4_u8, ADD, true},
        // Absolute,Y    ADC $4400,Y   $79  3   4+
        {INS_ADC_ABSY, "ADC", AddressMode::ABSY, 3_u8, 4_u8, ADD, false},
        {INS_ADC_ABSY, "ADC_crosspage", AddressMode::ABSY, 3_u8, 4_u8, ADD, true},
        // Indirect,X    ADC ($44,X)   $61  2   6
        {INS_ADC_INDX, "ADC", AddressMode::INDX, 2_u8, 6_u8, ADD, false},
        // Indirect,Y    ADC ($44),Y   $71  2   5+
        {INS_ADC_INDY, "ADC", AddressMode::INDY, 2_u8, 5_u8, ADD, false},
        {INS_ADC_INDY, "ADC_crosspage", AddressMode::INDY, 2_u8, 5_u8, ADD, true},
        // Immediate     SBC #$44      $E9  2   2
        {INS_SBC, "SBC", AddressMode::IM, 2_u8, 2_u8, SBC, false},
        // Absolute      SBC $4400     $ED  3   4
        {INS_SBC_ABS, "SBC", AddressMode::ABS, 3_u8, 4_u8, SBC, false},
        // Zero Page     SBC $44       $E5  2   3
        {INS_SBC_ZP, "SBC", AddressMode::ZP, 2_u8, 3_u8, SBC, false},
        // Zero Page,X   SBC $44,X     $F5  2   4
        {INS_SBC_ZPX, "SBC", AddressMode::ZPX, 2_u8, 4_u8, SBC, false},
        // Absolute,X    SBC $4400,X   $FD  3   4+
        {INS_SBC_ABSX, "SBC", AddressMode::ABSX, 3_u8, 4_u8, SBC, false},
        {INS_SBC_ABSX, "SBC_crosspage", AddressMode::ABSX, 3_u8, 4_u8, SBC, true},
        // Absolute,Y    SBC $4400,Y   $F9  3   4+
        {INS_SBC_ABSY, "SBC", AddressMode::ABSY, 3_u8, 4_u8, SBC, false},
        {INS_SBC_ABSY, "SBC_crosspage", AddressMode::ABSY, 3_u8, 4_u8, SBC, true},
        // Indirect,X    SBC ($44,X)   $E1  2   6
        {INS_SBC_INDX, "SBC", AddressMode::INDX, 2_u8, 6_u8, SBC, false},
        // Indirect,Y    SBC ($44),Y   $F1  2   5+
        {INS_SBC_INDY, "SBC", AddressMode::INDY, 2_u8, 5_u8, SBC, false},
        {INS_SBC_INDY, "SBC_crosspage", AddressMode::INDY, 2_u8, 5_u8, SBC, true},
    };
}

INSTANTIATE_TEST_SUITE_P(, ArithmeticTest, ::testing::ValuesIn(GetTestCases()),
                         GenTestNameFunc());

} // namespace
} // namespace emu::emu6502::test