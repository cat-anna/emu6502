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

    constexpr auto ZP = &Cpu6502::GetZeroPageAddress;
    constexpr auto ZPX = &Cpu6502::GetZeroPageIndirectAddressWithX;
    constexpr auto ABS = &Cpu6502::GetAbsoluteAddress;
    constexpr auto ABSX = &Cpu6502::GetAddressAbsoluteIndexedWithX;
    constexpr auto ABSY = &Cpu6502::GetAddressAbsoluteIndexedWithY;
    constexpr auto INDX = &Cpu6502::GetAddresZeroPageIndexedIndirectWithX;
    constexpr auto INDY = &Cpu6502::GetAddresZeroPageIndirectIndexedWithY;

    constexpr auto Fetch_IM = &Cpu6502::FetchNextByte;
    constexpr auto Fetch_ZP = &Cpu6502::FetchMemory<ZP>;
    constexpr auto Fetch_ZPX = &Cpu6502::FetchMemory<ZPX>;
    constexpr auto Fetch_ABS = &Cpu6502::FetchMemory<ABS>;
    constexpr auto Fetch_ABSX = &Cpu6502::FetchMemory<ABSX>;
    constexpr auto Fetch_ABSY = &Cpu6502::FetchMemory<ABSY>;
    constexpr auto Fetch_INDX = &Cpu6502::FetchMemory<INDX>;
    constexpr auto Fetch_INDY = &Cpu6502::FetchMemory<INDY>;

    //DEC
    r[INS_DEC_ABS] = &Cpu6502::Inc<&Cpu6502::GetAbsoluteAddress, -1>;
    r[INS_DEC_ABSX] = &Cpu6502::Inc<&Cpu6502::GetAddressAbsoluteIndexedWithX, -1>;
    r[INS_DEC_ZP] = &Cpu6502::Inc<&Cpu6502::GetZeroPageAddress, -1>;
    r[INS_DEC_ZPX] = &Cpu6502::Inc<&Cpu6502::GetZeroPageIndirectAddressWithX, -1>;

    //INC
    r[INS_INC_ABS] = &Cpu6502::Inc<&Cpu6502::GetAbsoluteAddress, 1>;
    r[INS_INC_ABSX] = &Cpu6502::Inc<&Cpu6502::GetAddressAbsoluteIndexedWithX, 1>;
    r[INS_INC_ZP] = &Cpu6502::Inc<&Cpu6502::GetZeroPageAddress, 1>;
    r[INS_INC_ZPX] = &Cpu6502::Inc<&Cpu6502::GetZeroPageIndirectAddressWithX, 1>;

    //LDA
    r[INS_LDA_ABS] = &Cpu6502::Reg8LD<&Registers::a, &Cpu6502::GetAbsoluteAddress>;
    r[INS_LDA_ABSX] = &Cpu6502::Reg8LD<&Registers::a, &Cpu6502::GetAddressAbsoluteIndexedWithX>;
    r[INS_LDA_ABSY] = &Cpu6502::Reg8LD<&Registers::a, &Cpu6502::GetAddressAbsoluteIndexedWithY>;
    r[INS_LDA_IM] = &Cpu6502::Reg8LD_raw<&Registers::a>;
    r[INS_LDA_ZP] = &Cpu6502::Reg8LD<&Registers::a, &Cpu6502::GetZeroPageAddress>;
    r[INS_LDA_INDX] = &Cpu6502::Reg8LD<&Registers::a, &Cpu6502::GetAddresZeroPageIndexedIndirectWithX>;
    r[INS_LDA_ZPX] = &Cpu6502::Reg8LD<&Registers::a, &Cpu6502::GetZeroPageIndirectAddressWithX>;
    r[INS_LDA_INDY] = &Cpu6502::Reg8LD<&Registers::a, &Cpu6502::GetAddresZeroPageIndirectIndexedWithY>;
    // r[INS_LDA_INDZP] = &Cpu6502::Reg8LD<&Registers::a, &Cpu6502::GetZeroPageIndirectAddress>;

    //LDX
    r[INS_LDX_ABS] = &Cpu6502::Reg8LD<&Registers::x, &Cpu6502::GetAbsoluteAddress>;
    r[INS_LDX_ABSY] = &Cpu6502::Reg8LD<&Registers::x, &Cpu6502::GetAddressAbsoluteIndexedWithY>;
    r[INS_LDX_IM] = &Cpu6502::Reg8LD_raw<&Registers::x>;
    r[INS_LDX_ZP] = &Cpu6502::Reg8LD<&Registers::x, &Cpu6502::GetZeroPageAddress>;
    r[INS_LDX_ZPY] = &Cpu6502::Reg8LD<&Registers::x, &Cpu6502::GetZeroPageIndirectAddressWithY>;

    //LDY
    r[INS_LDY_ABS] = &Cpu6502::Reg8LD<&Registers::y, &Cpu6502::GetAbsoluteAddress>;
    r[INS_LDY_ABSX] = &Cpu6502::Reg8LD<&Registers::y, &Cpu6502::GetAddressAbsoluteIndexedWithX>;
    r[INS_LDY_IM] = &Cpu6502::Reg8LD_raw<&Registers::y>;
    r[INS_LDY_ZP] = &Cpu6502::Reg8LD<&Registers::y, &Cpu6502::GetZeroPageAddress>;
    r[INS_LDY_ZPX] = &Cpu6502::Reg8LD<&Registers::y, &Cpu6502::GetZeroPageIndirectAddressWithX>;

    //STA
    r[INS_STA_ZP] = &Cpu6502::Reg8ST<&Registers::a, &Cpu6502::GetZeroPageAddress>;
    r[INS_STA_ZPX] = &Cpu6502::Reg8ST<&Registers::a, &Cpu6502::GetZeroPageIndirectAddressWithX>;
    r[INS_STA_ABS] = &Cpu6502::Reg8ST<&Registers::a, &Cpu6502::GetAbsoluteAddress>;
    r[INS_STA_ABSX] = &Cpu6502::Reg8ST<&Registers::a, &Cpu6502::GetAddressAbsoluteIndexedWithX>;
    r[INS_STA_ABSY] = &Cpu6502::Reg8ST<&Registers::a, &Cpu6502::GetAddressAbsoluteIndexedWithY>;
    r[INS_STA_INDX] = &Cpu6502::Reg8ST<&Registers::a, &Cpu6502::GetAddresZeroPageIndexedIndirectWithX>;
    r[INS_STA_INDY] = &Cpu6502::Reg8ST<&Registers::a, &Cpu6502::GetAddresZeroPageIndirectIndexedWithY>;
    // r[INS_STA_INDZP] = &Cpu6502::Reg8ST<&Registers::a, &Cpu6502::GetZeroPageIndirectAddress>;

    //STX
    r[INS_STX_ZP] = &Cpu6502::Reg8ST<&Registers::x, &Cpu6502::GetZeroPageAddress>;
    r[INS_STX_ZPY] = &Cpu6502::Reg8ST<&Registers::x, &Cpu6502::GetZeroPageIndirectAddressWithY>;
    r[INS_STX_ABS] = &Cpu6502::Reg8ST<&Registers::x, &Cpu6502::GetAbsoluteAddress>;

    //STY
    r[INS_STY_ZP] = &Cpu6502::Reg8ST<&Registers::y, &Cpu6502::GetZeroPageAddress>;
    r[INS_STY_ZPX] = &Cpu6502::Reg8ST<&Registers::y, &Cpu6502::GetZeroPageIndirectAddressWithX>;
    r[INS_STY_ABS] = &Cpu6502::Reg8ST<&Registers::y, &Cpu6502::GetAbsoluteAddress>;

    //Transfer Registers
    r[INS_TAX] = &Cpu6502::Reg8Transfer<&Registers::a, &Registers::x>;
    r[INS_TAY] = &Cpu6502::Reg8Transfer<&Registers::a, &Registers::y>;
    r[INS_TXA] = &Cpu6502::Reg8Transfer<&Registers::x, &Registers::a>;
    r[INS_TYA] = &Cpu6502::Reg8Transfer<&Registers::y, &Registers::a>;

    //inc-dec registers
    r[INS_INY] = &Cpu6502::Reg8Inc<&Registers::y, 1>;
    r[INS_INX] = &Cpu6502::Reg8Inc<&Registers::x, 1>;
    // r[INS_INC_ACC] = &Cpu6502::Reg8Inc<&Registers::a, 1>;
    r[INS_DEY] = &Cpu6502::Reg8Inc<&Registers::y, -1>;
    r[INS_DEX] = &Cpu6502::Reg8Inc<&Registers::x, -1>;
    // r[INS_DEC_ACC] = &Cpu6502::Reg8Inc<&Registers::a, -1>;

    //Arithmetic
    r[INS_ADC] = &Cpu6502::Reg8Adc_raw<>;
    r[INS_ADC_ABS] = &Cpu6502::Reg8Adc<&Cpu6502::GetAbsoluteAddress>;
    r[INS_ADC_ZP] = &Cpu6502::Reg8Adc<&Cpu6502::GetZeroPageAddress>;
    r[INS_ADC_ZPX] = &Cpu6502::Reg8Adc<&Cpu6502::GetZeroPageIndirectAddressWithX>;
    r[INS_ADC_ABSX] = &Cpu6502::Reg8Adc<&Cpu6502::GetAddressAbsoluteIndexedWithX>;
    r[INS_ADC_ABSY] = &Cpu6502::Reg8Adc<&Cpu6502::GetAddressAbsoluteIndexedWithY>;
    r[INS_ADC_INDX] = &Cpu6502::Reg8Adc<&Cpu6502::GetAddresZeroPageIndexedIndirectWithX>;
    r[INS_ADC_INDY] = &Cpu6502::Reg8Adc<&Cpu6502::GetAddresZeroPageIndirectIndexedWithY>;
    r[INS_SBC] = &Cpu6502::Reg8Sbc_raw<>;
    r[INS_SBC_ABS] = &Cpu6502::Reg8Sbc<&Cpu6502::GetAbsoluteAddress>;
    r[INS_SBC_ZP] = &Cpu6502::Reg8Sbc<&Cpu6502::GetZeroPageAddress>;
    r[INS_SBC_ZPX] = &Cpu6502::Reg8Sbc<&Cpu6502::GetZeroPageIndirectAddressWithX>;
    r[INS_SBC_ABSX] = &Cpu6502::Reg8Sbc<&Cpu6502::GetAddressAbsoluteIndexedWithX>;
    r[INS_SBC_ABSY] = &Cpu6502::Reg8Sbc<&Cpu6502::GetAddressAbsoluteIndexedWithY>;
    r[INS_SBC_INDX] = &Cpu6502::Reg8Sbc<&Cpu6502::GetAddresZeroPageIndexedIndirectWithX>;
    r[INS_SBC_INDY] = &Cpu6502::Reg8Sbc<&Cpu6502::GetAddresZeroPageIndirectIndexedWithY>;

    // Register Comparison
    r[INS_CMP] = &Cpu6502::Reg8Compare<&Registers::a, Fetch_IM>;
    r[INS_CMP_ZP] = &Cpu6502::Reg8Compare<&Registers::a, Fetch_ZP>;
    r[INS_CMP_ZPX] = &Cpu6502::Reg8Compare<&Registers::a, Fetch_ZPX>;
    r[INS_CMP_ABS] = &Cpu6502::Reg8Compare<&Registers::a, Fetch_ABS>;
    r[INS_CMP_ABSX] = &Cpu6502::Reg8Compare<&Registers::a, Fetch_ABSX>;
    r[INS_CMP_ABSY] = &Cpu6502::Reg8Compare<&Registers::a, Fetch_ABSY>;
    r[INS_CMP_INDY] = &Cpu6502::Reg8Compare<&Registers::a, Fetch_INDY>;
    r[INS_CMP_INDX] = &Cpu6502::Reg8Compare<&Registers::a, Fetch_INDX>;
    r[INS_CPX] = &Cpu6502::Reg8Compare<&Registers::x, Fetch_IM>;
    r[INS_CPY] = &Cpu6502::Reg8Compare<&Registers::y, Fetch_IM>;
    r[INS_CPX_ZP] = &Cpu6502::Reg8Compare<&Registers::x, Fetch_ZP>;
    r[INS_CPY_ZP] = &Cpu6502::Reg8Compare<&Registers::y, Fetch_ZP>;
    r[INS_CPX_ABS] = &Cpu6502::Reg8Compare<&Registers::x, Fetch_ABS>;
    r[INS_CPY_ABS] = &Cpu6502::Reg8Compare<&Registers::y, Fetch_ABS>;

    //branches
    r[INS_BCC] = &Cpu6502::Branch<Flags::Carry, false>;
    r[INS_BCS] = &Cpu6502::Branch<Flags::Carry, true>;
    r[INS_BEQ] = &Cpu6502::Branch<Flags::Zero, true>;
    r[INS_BNE] = &Cpu6502::Branch<Flags::Zero, false>;
    r[INS_BMI] = &Cpu6502::Branch<Flags::Negative, true>;
    r[INS_BPL] = &Cpu6502::Branch<Flags::Negative, false>;
    r[INS_BVC] = &Cpu6502::Branch<Flags::Overflow, false>;
    r[INS_BVS] = &Cpu6502::Branch<Flags::Overflow, true>;
    // jumps/calls
    r[INS_JMP_ABS] = &Cpu6502::JumpABS;
    r[INS_JMP_IND] = &Cpu6502::JumpIND;
    r[INS_JSR] = &Cpu6502::JSR;
    r[INS_RTS] = &Cpu6502::RTS;
    r[INS_RTI] = &Cpu6502::RTI;

    //Logical
    r[INS_AND_IM] = &Cpu6502::LogicalOp<&Op::AND, Fetch_IM>;
    r[INS_AND_ZP] = &Cpu6502::LogicalOp<&Op::AND, Fetch_ZP>;
    r[INS_AND_ZPX] = &Cpu6502::LogicalOp<&Op::AND, Fetch_ZPX>;
    r[INS_AND_ABS] = &Cpu6502::LogicalOp<&Op::AND, Fetch_ABS>;
    r[INS_AND_ABSX] = &Cpu6502::LogicalOp<&Op::AND, Fetch_ABSX>;
    r[INS_AND_ABSY] = &Cpu6502::LogicalOp<&Op::AND, Fetch_ABSY>;
    r[INS_AND_INDX] = &Cpu6502::LogicalOp<&Op::AND, Fetch_INDX>;
    r[INS_AND_INDY] = &Cpu6502::LogicalOp<&Op::AND, Fetch_INDY>;
    r[INS_ORA_IM] = &Cpu6502::LogicalOp<&Op::ORA, Fetch_IM>;
    r[INS_ORA_ZP] = &Cpu6502::LogicalOp<&Op::ORA, Fetch_ZP>;
    r[INS_ORA_ZPX] = &Cpu6502::LogicalOp<&Op::ORA, Fetch_ZPX>;
    r[INS_ORA_ABS] = &Cpu6502::LogicalOp<&Op::ORA, Fetch_ABS>;
    r[INS_ORA_ABSX] = &Cpu6502::LogicalOp<&Op::ORA, Fetch_ABSX>;
    r[INS_ORA_ABSY] = &Cpu6502::LogicalOp<&Op::ORA, Fetch_ABSY>;
    r[INS_ORA_INDX] = &Cpu6502::LogicalOp<&Op::ORA, Fetch_INDX>;
    r[INS_ORA_INDY] = &Cpu6502::LogicalOp<&Op::ORA, Fetch_INDY>;
    r[INS_EOR_IM] = &Cpu6502::LogicalOp<&Op::XOR, Fetch_IM>;
    r[INS_EOR_ZP] = &Cpu6502::LogicalOp<&Op::XOR, Fetch_ZP>;
    r[INS_EOR_ZPX] = &Cpu6502::LogicalOp<&Op::XOR, Fetch_ZPX>;
    r[INS_EOR_ABS] = &Cpu6502::LogicalOp<&Op::XOR, Fetch_ABS>;
    r[INS_EOR_ABSX] = &Cpu6502::LogicalOp<&Op::XOR, Fetch_ABSX>;
    r[INS_EOR_ABSY] = &Cpu6502::LogicalOp<&Op::XOR, Fetch_ABSY>;
    r[INS_EOR_INDX] = &Cpu6502::LogicalOp<&Op::XOR, Fetch_INDX>;
    r[INS_EOR_INDY] = &Cpu6502::LogicalOp<&Op::XOR, Fetch_INDY>;

    //status flag changes
    r[INS_CLC] = &Cpu6502::SetFlag<Flags::Carry, false>;
    r[INS_SEC] = &Cpu6502::SetFlag<Flags::Carry, true>;
    r[INS_CLD] = &Cpu6502::SetFlag<Flags::DecimalMode, false>;
    r[INS_SED] = &Cpu6502::SetFlag<Flags::DecimalMode, true>;
    r[INS_CLI] = &Cpu6502::SetFlag<Flags::IRQB, false>;
    r[INS_SEI] = &Cpu6502::SetFlag<Flags::IRQB, true>;
    r[INS_CLV] = &Cpu6502::SetFlag<Flags::Overflow, false>;

    // shifts
    r[INS_ASL] = &Cpu6502::ShiftRegister<&Registers::a, &Op::ASL>;
    r[INS_LSR] = &Cpu6502::ShiftRegister<&Registers::a, &Op::LSR>;
    r[INS_ROL] = &Cpu6502::ShiftRegister<&Registers::a, &Op::ROL>;
    r[INS_ROR] = &Cpu6502::ShiftRegister<&Registers::a, &Op::ROR>;

    r[INS_ASL_ZP] = &Cpu6502::ShiftMemory<&Op::ASL, ZP>;
    r[INS_ASL_ZPX] = &Cpu6502::ShiftMemory<&Op::ASL, ZPX>;
    r[INS_ASL_ABS] = &Cpu6502::ShiftMemory<&Op::ASL, ABS>;
    r[INS_ASL_ABSX] = &Cpu6502::ShiftMemory<&Op::ASL, ABSX>;
    r[INS_LSR_ZP] = &Cpu6502::ShiftMemory<&Op::LSR, ZP>;
    r[INS_LSR_ZPX] = &Cpu6502::ShiftMemory<&Op::LSR, ZPX>;
    r[INS_LSR_ABS] = &Cpu6502::ShiftMemory<&Op::LSR, ABS>;
    r[INS_LSR_ABSX] = &Cpu6502::ShiftMemory<&Op::LSR, ABSX>;
    r[INS_ROL_ZP] = &Cpu6502::ShiftMemory<&Op::ROL, ZP>;
    r[INS_ROL_ZPX] = &Cpu6502::ShiftMemory<&Op::ROL, ZPX>;
    r[INS_ROL_ABS] = &Cpu6502::ShiftMemory<&Op::ROL, ABS>;
    r[INS_ROL_ABSX] = &Cpu6502::ShiftMemory<&Op::ROL, ABSX>;
    r[INS_ROR_ZP] = &Cpu6502::ShiftMemory<&Op::ROR, ZP>;
    r[INS_ROR_ZPX] = &Cpu6502::ShiftMemory<&Op::ROR, ZPX>;
    r[INS_ROR_ABS] = &Cpu6502::ShiftMemory<&Op::ROR, ABS>;
    r[INS_ROR_ABSX] = &Cpu6502::ShiftMemory<&Op::ROR, ABSX>;

    // stack
    r[INS_TSX] = &Cpu6502::Reg8Transfer<&Registers::stack_pointer, &Registers::x>;
    r[INS_TXS] = &Cpu6502::Reg8Transfer<&Registers::x, &Registers::stack_pointer, false>;

    r[INS_PHA] = &Cpu6502::StackPush<&Registers::a>;
    r[INS_PLA] = &Cpu6502::StackPull<&Registers::a>;
    r[INS_PHP] = &Cpu6502::PushFlags;
    r[INS_PLP] = &Cpu6502::PullFlags;

    //BIT
    r[INS_BIT_ZP] = &Cpu6502::BitOp<&Op::AND, Fetch_ZP>;
    r[INS_BIT_ABS] = &Cpu6502::BitOp<&Op::AND, Fetch_ABS>;

    //misc
    r[INS_NOP] = &Cpu6502::NOP;
    r[INS_BRK] = &Cpu6502::BRK;

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