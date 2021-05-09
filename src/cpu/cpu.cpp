#include "cpu.hpp"
#include "opcode.hpp"
#include <fmt/format.h>

namespace emu6502::cpu {

namespace {

struct Op {
    static uint8_t AND(uint8_t a, uint8_t b) { return a & b; }
    static uint8_t ORA(uint8_t a, uint8_t b) { return a | b; }
    static uint8_t XOR(uint8_t a, uint8_t b) { return a ^ b; }

    static std::tuple<uint8_t, bool> ASL(uint8_t v, bool carry) { //
        return std::make_tuple((v << 1), (v & 0x80) != 0);
    }
    static std::tuple<uint8_t, bool> LSR(uint8_t v, bool carry) { //
        return std::make_tuple((v >> 1), (v & 0x01) != 0);
    }
    static std::tuple<uint8_t, bool> ROL(uint8_t v, bool carry) { //
        return std::make_tuple((v << 1) | (carry ? 0x01 : 0), (v & 0x80) != 0);
    }
    static std::tuple<uint8_t, bool> ROR(uint8_t v, bool carry) { //
        return std::make_tuple((v >> 1) | (carry ? 0x80 : 0), (v & 0x01) != 0);
    }
};

} // namespace

Cpu6502::InstructionHandlerArray Cpu6502::InitInstructionHandlerArray() {
    InstructionHandlerArray r;
    r.fill(nullptr);

    using namespace opcode;
    using Flags = Registers::Flags;

    constexpr auto ZP = &GetZeroPageAddress;
    constexpr auto ZPX = &GetZeroPageIndirectAddressWithX;
    constexpr auto ABS = &GetAbsoluteAddress;
    constexpr auto ABSX = &GetAddressAbsoluteIndexedWithX;
    constexpr auto ABSY = &GetAddressAbsoluteIndexedWithY;
    constexpr auto INDX = &GetAddresZeroPageIndexedIndirectWithX;
    constexpr auto INDY = &GetAddresZeroPageIndirectIndexedWithY;

    constexpr auto Fetch_IM = &Cpu6502::FetchNextByte;
    constexpr auto Fetch_ZP = &Cpu6502::FetchMemory<ZP>;
    constexpr auto Fetch_ZPX = &Cpu6502::FetchMemory<ZPX>;
    constexpr auto Fetch_ABS = &Cpu6502::FetchMemory<ABS>;
    constexpr auto Fetch_ABSX = &Cpu6502::FetchMemory<ABSX>;
    constexpr auto Fetch_ABSY = &Cpu6502::FetchMemory<ABSY>;
    constexpr auto Fetch_INDX = &Cpu6502::FetchMemory<INDX>;
    constexpr auto Fetch_INDY = &Cpu6502::FetchMemory<INDY>;

    //DEC
    r[INS_DEC_ABS] = &Inc<&GetAbsoluteAddress, -1>;
    r[INS_DEC_ABSX] = &Inc<&GetAddressAbsoluteIndexedWithX, -1>;
    r[INS_DEC_ZP] = &Inc<&GetZeroPageAddress, -1>;
    r[INS_DEC_ZPX] = &Inc<&GetZeroPageIndirectAddressWithX, -1>;

    //INC
    r[INS_INC_ABS] = &Inc<&GetAbsoluteAddress, 1>;
    r[INS_INC_ABSX] = &Inc<&GetAddressAbsoluteIndexedWithX, 1>;
    r[INS_INC_ZP] = &Inc<&GetZeroPageAddress, 1>;
    r[INS_INC_ZPX] = &Inc<&GetZeroPageIndirectAddressWithX, 1>;

    //LDA
    r[INS_LDA_ABS] = &Reg8LD<&Registers::a, &GetAbsoluteAddress>;
    r[INS_LDA_ABSX] = &Reg8LD<&Registers::a, &GetAddressAbsoluteIndexedWithX>;
    r[INS_LDA_ABSY] = &Reg8LD<&Registers::a, &GetAddressAbsoluteIndexedWithY>;
    r[INS_LDA_IM] = &Reg8LD_raw<&Registers::a>;
    r[INS_LDA_ZP] = &Reg8LD<&Registers::a, &GetZeroPageAddress>;
    r[INS_LDA_INDX] = &Reg8LD<&Registers::a, &GetAddresZeroPageIndexedIndirectWithX>;
    r[INS_LDA_ZPX] = &Reg8LD<&Registers::a, &GetZeroPageIndirectAddressWithX>;
    r[INS_LDA_INDY] = &Reg8LD<&Registers::a, &GetAddresZeroPageIndirectIndexedWithY>;
    // r[INS_LDA_INDZP] = &Reg8LD<&Registers::a, &GetZeroPageIndirectAddress>;

    //LDX
    r[INS_LDX_ABS] = &Reg8LD<&Registers::x, &GetAbsoluteAddress>;
    r[INS_LDX_ABSY] = &Reg8LD<&Registers::x, &GetAddressAbsoluteIndexedWithY>;
    r[INS_LDX_IM] = &Reg8LD_raw<&Registers::x>;
    r[INS_LDX_ZP] = &Reg8LD<&Registers::x, &GetZeroPageAddress>;
    r[INS_LDX_ZPY] = &Reg8LD<&Registers::x, &GetZeroPageIndirectAddressWithY>;

    //LDY
    r[INS_LDY_ABS] = &Reg8LD<&Registers::y, &GetAbsoluteAddress>;
    r[INS_LDY_ABSX] = &Reg8LD<&Registers::y, &GetAddressAbsoluteIndexedWithX>;
    r[INS_LDY_IM] = &Reg8LD_raw<&Registers::y>;
    r[INS_LDY_ZP] = &Reg8LD<&Registers::y, &GetZeroPageAddress>;
    r[INS_LDY_ZPX] = &Reg8LD<&Registers::y, &GetZeroPageIndirectAddressWithX>;

    //STA
    r[INS_STA_ZP] = &Reg8ST<&Registers::a, &GetZeroPageAddress>;
    r[INS_STA_ZPX] = &Reg8ST<&Registers::a, &GetZeroPageIndirectAddressWithX>;
    r[INS_STA_ABS] = &Reg8ST<&Registers::a, &GetAbsoluteAddress>;
    r[INS_STA_ABSX] = &Reg8ST<&Registers::a, &GetAddressAbsoluteIndexedWithX>;
    r[INS_STA_ABSY] = &Reg8ST<&Registers::a, &GetAddressAbsoluteIndexedWithY>;
    r[INS_STA_INDX] = &Reg8ST<&Registers::a, &GetAddresZeroPageIndexedIndirectWithX>;
    r[INS_STA_INDY] = &Reg8ST<&Registers::a, &GetAddresZeroPageIndirectIndexedWithY>;
    // r[INS_STA_INDZP] = &Reg8ST<&Registers::a, &GetZeroPageIndirectAddress>;

    //STX
    r[INS_STX_ZP] = &Reg8ST<&Registers::x, &GetZeroPageAddress>;
    r[INS_STX_ZPY] = &Reg8ST<&Registers::x, &GetZeroPageIndirectAddressWithY>;
    r[INS_STX_ABS] = &Reg8ST<&Registers::x, &GetAbsoluteAddress>;

    //STY
    r[INS_STY_ZP] = &Reg8ST<&Registers::y, &GetZeroPageAddress>;
    r[INS_STY_ZPX] = &Reg8ST<&Registers::y, &GetZeroPageIndirectAddressWithX>;
    r[INS_STY_ABS] = &Reg8ST<&Registers::y, &GetAbsoluteAddress>;

    //Transfer Registers
    r[INS_TAX] = &Reg8Transfer<&Registers::a, &Registers::x>;
    r[INS_TAY] = &Reg8Transfer<&Registers::a, &Registers::y>;
    r[INS_TXA] = &Reg8Transfer<&Registers::x, &Registers::a>;
    r[INS_TYA] = &Reg8Transfer<&Registers::y, &Registers::a>;

    //inc-dec registers
    r[INS_INY] = &Reg8Inc<&Registers::y, 1>;
    r[INS_INX] = &Reg8Inc<&Registers::x, 1>;
    // r[INS_INC_ACC] = &Reg8Inc<&Registers::a, 1>;
    r[INS_DEY] = &Reg8Inc<&Registers::y, -1>;
    r[INS_DEX] = &Reg8Inc<&Registers::x, -1>;
    // r[INS_DEC_ACC] = &Reg8Inc<&Registers::a, -1>;

    //Arithmetic
    r[INS_ADC] = &Reg8Adc_raw<>;
    r[INS_ADC_ABS] = &Reg8Adc<&GetAbsoluteAddress>;
    r[INS_ADC_ZP] = &Reg8Adc<&GetZeroPageAddress>;
    r[INS_ADC_ZPX] = &Reg8Adc<&GetZeroPageIndirectAddressWithX>;
    r[INS_ADC_ABSX] = &Reg8Adc<&GetAddressAbsoluteIndexedWithX>;
    r[INS_ADC_ABSY] = &Reg8Adc<&GetAddressAbsoluteIndexedWithY>;
    r[INS_ADC_INDX] = &Reg8Adc<&GetAddresZeroPageIndexedIndirectWithX>;
    r[INS_ADC_INDY] = &Reg8Adc<&GetAddresZeroPageIndirectIndexedWithY>;
    r[INS_SBC] = &Reg8Sbc_raw<>;
    r[INS_SBC_ABS] = &Reg8Sbc<&GetAbsoluteAddress>;
    r[INS_SBC_ZP] = &Reg8Sbc<&GetZeroPageAddress>;
    r[INS_SBC_ZPX] = &Reg8Sbc<&GetZeroPageIndirectAddressWithX>;
    r[INS_SBC_ABSX] = &Reg8Sbc<&GetAddressAbsoluteIndexedWithX>;
    r[INS_SBC_ABSY] = &Reg8Sbc<&GetAddressAbsoluteIndexedWithY>;
    r[INS_SBC_INDX] = &Reg8Sbc<&GetAddresZeroPageIndexedIndirectWithX>;
    r[INS_SBC_INDY] = &Reg8Sbc<&GetAddresZeroPageIndirectIndexedWithY>;

    // Register Comparison
    r[INS_CMP] = &Reg8Compare<&Registers::a, Fetch_IM>;
    r[INS_CMP_ZP] = &Reg8Compare<&Registers::a, Fetch_ZP>;
    r[INS_CMP_ZPX] = &Reg8Compare<&Registers::a, Fetch_ZPX>;
    r[INS_CMP_ABS] = &Reg8Compare<&Registers::a, Fetch_ABS>;
    r[INS_CMP_ABSX] = &Reg8Compare<&Registers::a, Fetch_ABSX>;
    r[INS_CMP_ABSY] = &Reg8Compare<&Registers::a, Fetch_ABSY>;
    r[INS_CMP_INDY] = &Reg8Compare<&Registers::a, Fetch_INDY>;
    r[INS_CMP_INDX] = &Reg8Compare<&Registers::a, Fetch_INDX>;
    r[INS_CPX] = &Reg8Compare<&Registers::x, Fetch_IM>;
    r[INS_CPY] = &Reg8Compare<&Registers::y, Fetch_IM>;
    r[INS_CPX_ZP] = &Reg8Compare<&Registers::x, Fetch_ZP>;
    r[INS_CPY_ZP] = &Reg8Compare<&Registers::y, Fetch_ZP>;
    r[INS_CPX_ABS] = &Reg8Compare<&Registers::x, Fetch_ABS>;
    r[INS_CPY_ABS] = &Reg8Compare<&Registers::y, Fetch_ABS>;

    //branches
    r[INS_BCC] = &Branch<Flags::Carry, false>;
    r[INS_BCS] = &Branch<Flags::Carry, true>;
    r[INS_BEQ] = &Branch<Flags::Zero, true>;
    r[INS_BNE] = &Branch<Flags::Zero, false>;
    r[INS_BMI] = &Branch<Flags::Negative, true>;
    r[INS_BPL] = &Branch<Flags::Negative, false>;
    r[INS_BVC] = &Branch<Flags::Overflow, false>;
    r[INS_BVS] = &Branch<Flags::Overflow, true>;
    // jumps/calls
    r[INS_JMP_ABS] = &JumpABS;
    r[INS_JMP_IND] = &JumpIND;
    r[INS_JSR] = &JSR;
    r[INS_RTS] = &RTS;
    r[INS_RTI] = &RTI;

    //Logical
    r[INS_AND_IM] = &LogicalOp<&Op::AND, Fetch_IM>;
    r[INS_AND_ZP] = &LogicalOp<&Op::AND, Fetch_ZP>;
    r[INS_AND_ZPX] = &LogicalOp<&Op::AND, Fetch_ZPX>;
    r[INS_AND_ABS] = &LogicalOp<&Op::AND, Fetch_ABS>;
    r[INS_AND_ABSX] = &LogicalOp<&Op::AND, Fetch_ABSX>;
    r[INS_AND_ABSY] = &LogicalOp<&Op::AND, Fetch_ABSY>;
    r[INS_AND_INDX] = &LogicalOp<&Op::AND, Fetch_INDX>;
    r[INS_AND_INDY] = &LogicalOp<&Op::AND, Fetch_INDY>;
    r[INS_ORA_IM] = &LogicalOp<&Op::ORA, Fetch_IM>;
    r[INS_ORA_ZP] = &LogicalOp<&Op::ORA, Fetch_ZP>;
    r[INS_ORA_ZPX] = &LogicalOp<&Op::ORA, Fetch_ZPX>;
    r[INS_ORA_ABS] = &LogicalOp<&Op::ORA, Fetch_ABS>;
    r[INS_ORA_ABSX] = &LogicalOp<&Op::ORA, Fetch_ABSX>;
    r[INS_ORA_ABSY] = &LogicalOp<&Op::ORA, Fetch_ABSY>;
    r[INS_ORA_INDX] = &LogicalOp<&Op::ORA, Fetch_INDX>;
    r[INS_ORA_INDY] = &LogicalOp<&Op::ORA, Fetch_INDY>;
    r[INS_EOR_IM] = &LogicalOp<&Op::XOR, Fetch_IM>;
    r[INS_EOR_ZP] = &LogicalOp<&Op::XOR, Fetch_ZP>;
    r[INS_EOR_ZPX] = &LogicalOp<&Op::XOR, Fetch_ZPX>;
    r[INS_EOR_ABS] = &LogicalOp<&Op::XOR, Fetch_ABS>;
    r[INS_EOR_ABSX] = &LogicalOp<&Op::XOR, Fetch_ABSX>;
    r[INS_EOR_ABSY] = &LogicalOp<&Op::XOR, Fetch_ABSY>;
    r[INS_EOR_INDX] = &LogicalOp<&Op::XOR, Fetch_INDX>;
    r[INS_EOR_INDY] = &LogicalOp<&Op::XOR, Fetch_INDY>;

    //status flag changes
    r[INS_CLC] = &SetFlag<Flags::Carry, false>;
    r[INS_SEC] = &SetFlag<Flags::Carry, true>;
    r[INS_CLD] = &SetFlag<Flags::DecimalMode, false>;
    r[INS_SED] = &SetFlag<Flags::DecimalMode, true>;
    r[INS_CLI] = &SetFlag<Flags::IRQB, false>;
    r[INS_SEI] = &SetFlag<Flags::IRQB, true>;
    r[INS_CLV] = &SetFlag<Flags::Overflow, false>;

    // shifts
    r[INS_ASL] = &ShiftRegister<&Registers::a, &Op::ASL>;
    r[INS_LSR] = &ShiftRegister<&Registers::a, &Op::LSR>;
    r[INS_ROL] = &ShiftRegister<&Registers::a, &Op::ROL>;
    r[INS_ROR] = &ShiftRegister<&Registers::a, &Op::ROR>;

    r[INS_ASL_ZP] = &ShiftMemory<&Op::ASL, ZP>;
    r[INS_ASL_ZPX] = &ShiftMemory<&Op::ASL, ZPX>;
    r[INS_ASL_ABS] = &ShiftMemory<&Op::ASL, ABS>;
    r[INS_ASL_ABSX] = &ShiftMemory<&Op::ASL, ABSX>;
    r[INS_LSR_ZP] = &ShiftMemory<&Op::LSR, ZP>;
    r[INS_LSR_ZPX] = &ShiftMemory<&Op::LSR, ZPX>;
    r[INS_LSR_ABS] = &ShiftMemory<&Op::LSR, ABS>;
    r[INS_LSR_ABSX] = &ShiftMemory<&Op::LSR, ABSX>;
    r[INS_ROL_ZP] = &ShiftMemory<&Op::ROL, ZP>;
    r[INS_ROL_ZPX] = &ShiftMemory<&Op::ROL, ZPX>;
    r[INS_ROL_ABS] = &ShiftMemory<&Op::ROL, ABS>;
    r[INS_ROL_ABSX] = &ShiftMemory<&Op::ROL, ABSX>;
    r[INS_ROR_ZP] = &ShiftMemory<&Op::ROR, ZP>;
    r[INS_ROR_ZPX] = &ShiftMemory<&Op::ROR, ZPX>;
    r[INS_ROR_ABS] = &ShiftMemory<&Op::ROR, ABS>;
    r[INS_ROR_ABSX] = &ShiftMemory<&Op::ROR, ABSX>;

    // stack
    r[INS_TSX] = &Reg8Transfer<&Registers::stack_pointer, &Registers::x>;
    r[INS_TXS] = &Reg8Transfer<&Registers::x, &Registers::stack_pointer, false>;

    r[INS_PHA] = &StackPush<&Registers::a>;
    r[INS_PLA] = &StackPull<&Registers::a>;
    r[INS_PHP] = &PushFlags;
    r[INS_PLP] = &PullFlags;

    //BIT
    r[INS_BIT_ZP] = &BitOp<&Op::AND, Fetch_ZP>;
    r[INS_BIT_ABS] = &BitOp<&Op::AND, Fetch_ABS>;

    //misc
    r[INS_NOP] = &NOP;
    r[INS_BRK] = &BRK;

    return r;
}

Cpu6502::Cpu6502() : instruction_handlers(InitInstructionHandlerArray()) {
}

void Cpu6502::ExecuteUntil(uint64_t cycle) {
    while (clock->CurrentCycle() < cycle) {
        ExecuteNextInstruction();
    }
}

void Cpu6502::ExecuteNextInstruction() {
    auto opcode = memory->Load(reg.program_counter);
    auto handler = instruction_handlers[opcode];
    if (handler == nullptr) {
        throw std::runtime_error(fmt::format("Invalid opcode {:02x} at address {:04x}", opcode, reg.program_counter));
    }
    reg.program_counter++;
    (this->*handler)();
}

} // namespace emu6502::cpu