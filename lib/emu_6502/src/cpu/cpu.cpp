#include "emu6502/cpu/cpu.hpp"
#include "emu6502/cpu/opcode.hpp"
#include "emu6502/instruction_set.hpp"
#include "instruction_functors.hpp"
#include "memory_addressing.hpp"

namespace emu::emu6502::cpu {

namespace {

InstructionHandlerArray GenInstructionHandlerArray(InstructionSet instruction_set) {
    InstructionHandlerArray r;
    r.fill(nullptr);

    using namespace opcode;
    using namespace instructions;
    using Flags = Registers::Flags;

    //LDA
    r[INS_LDA_ABS] = &Register8Load<&Registers::a, kFetchABS>;
    r[INS_LDA_ABSX] = &Register8Load<&Registers::a, kFetchFastABSX>;
    r[INS_LDA_ABSY] = &Register8Load<&Registers::a, kFetchFastABSY>;
    r[INS_LDA_IM] = &Register8Load<&Registers::a, kFetchIM>;
    r[INS_LDA_ZP] = &Register8Load<&Registers::a, kFetchZP>;
    r[INS_LDA_ZPX] = &Register8Load<&Registers::a, kFetchZPX>;
    r[INS_LDA_INDX] = &Register8Load<&Registers::a, kFetchINDX>;
    r[INS_LDA_INDY] = &Register8Load<&Registers::a, kFetchINDY>;
    // r[INS_LDA_INDZP] = &Register8Load<&Registers::a, &Cpu::GetZeroPageIndirectAddress>;

    //LDX
    r[INS_LDX_ABS] = &Register8Load<&Registers::x, kFetchABS>;
    r[INS_LDX_ABSY] = &Register8Load<&Registers::x, kFetchFastABSY>;
    r[INS_LDX_IM] = &Register8Load<&Registers::x, kFetchIM>;
    r[INS_LDX_ZP] = &Register8Load<&Registers::x, kFetchZP>;
    r[INS_LDX_ZPY] = &Register8Load<&Registers::x, kFetchZPY>;

    //LDY
    r[INS_LDY_ABS] = &Register8Load<&Registers::y, kFetchABS>;
    r[INS_LDY_ABSX] = &Register8Load<&Registers::y, kFetchFastABSX>;
    r[INS_LDY_IM] = &Register8Load<&Registers::y, kFetchIM>;
    r[INS_LDY_ZP] = &Register8Load<&Registers::y, kFetchZP>;
    r[INS_LDY_ZPX] = &Register8Load<&Registers::y, kFetchZPX>;

    //STA
    r[INS_STA_ZP] = &Register8Store<&Registers::a, kAddressZP>;
    r[INS_STA_ZPX] = &Register8Store<&Registers::a, kAddressZPX>;
    r[INS_STA_ABS] = &Register8Store<&Registers::a, kAddressABS>;
    r[INS_STA_ABSX] = &Register8Store<&Registers::a, kAddressABSX>;
    r[INS_STA_ABSY] = &Register8Store<&Registers::a, kAddressABSY>;
    r[INS_STA_INDX] = &Register8Store<&Registers::a, kAddressINDX>;
    r[INS_STA_INDY] = &Register8Store<&Registers::a, kAddressSlowINDY>;
    // r[INS_STA_INDZP] = &Register8Store<&Registers::a, &Cpu::kAddressGetZeroPageIndirectAddress>;

    //STX
    r[INS_STX_ZP] = &Register8Store<&Registers::x, kAddressZP>;
    r[INS_STX_ZPY] = &Register8Store<&Registers::x, kAddressZPY>;
    r[INS_STX_ABS] = &Register8Store<&Registers::x, kAddressABS>;

    //STY
    r[INS_STY_ZP] = &Register8Store<&Registers::y, kAddressZP>;
    r[INS_STY_ZPX] = &Register8Store<&Registers::y, kAddressZPX>;
    r[INS_STY_ABS] = &Register8Store<&Registers::y, kAddressABS>;

    //DEC
    r[INS_DEC_ABS] = &MemoryIncrement<kAddressABS, -1>;
    r[INS_DEC_ABSX] = &MemoryIncrement<kAddressABSX, -1>;
    r[INS_DEC_ZP] = &MemoryIncrement<kAddressZP, -1>;
    r[INS_DEC_ZPX] = &MemoryIncrement<kAddressZPX, -1>;

    //INC
    r[INS_INC_ABS] = &MemoryIncrement<kAddressABS, 1>;
    r[INS_INC_ABSX] = &MemoryIncrement<kAddressABSX, 1>;
    r[INS_INC_ZP] = &MemoryIncrement<kAddressZP, 1>;
    r[INS_INC_ZPX] = &MemoryIncrement<kAddressZPX, 1>;

    //Transfer Registers
    r[INS_TAX] = &Register8Transfer<&Registers::a, &Registers::x>;
    r[INS_TAY] = &Register8Transfer<&Registers::a, &Registers::y>;
    r[INS_TXA] = &Register8Transfer<&Registers::x, &Registers::a>;
    r[INS_TYA] = &Register8Transfer<&Registers::y, &Registers::a>;

    //inc-dec registers
    r[INS_INY] = &Register8Increment<&Registers::y, 1>;
    r[INS_INX] = &Register8Increment<&Registers::x, 1>;
    // r[INS_INC_ACC] = &Register8Increment<&Registers::a, 1>;
    r[INS_DEY] = &Register8Increment<&Registers::y, -1>;
    r[INS_DEX] = &Register8Increment<&Registers::x, -1>;
    // r[INS_DEC_ACC] = &Register8Increment<&Registers::a, -1>;

    //Arithmetic
    r[INS_ADC] = &ArithmeticOperation<kFetchIM, false>;
    r[INS_ADC_ABS] = &ArithmeticOperation<kFetchABS, false>;
    r[INS_ADC_ZP] = &ArithmeticOperation<kFetchZP, false>;
    r[INS_ADC_ZPX] = &ArithmeticOperation<kFetchZPX, false>;
    r[INS_ADC_ABSX] = &ArithmeticOperation<kFetchFastABSX, false>;
    r[INS_ADC_ABSY] = &ArithmeticOperation<kFetchFastABSY, false>;
    r[INS_ADC_INDX] = &ArithmeticOperation<kFetchINDX, false>;
    r[INS_ADC_INDY] = &ArithmeticOperation<kFetchINDY, false>;
    r[INS_SBC] = &ArithmeticOperation<kFetchIM, true>;
    r[INS_SBC_ABS] = &ArithmeticOperation<kFetchABS, true>;
    r[INS_SBC_ZP] = &ArithmeticOperation<kFetchZP, true>;
    r[INS_SBC_ZPX] = &ArithmeticOperation<kFetchZPX, true>;
    r[INS_SBC_ABSX] = &ArithmeticOperation<kFetchFastABSX, true>;
    r[INS_SBC_ABSY] = &ArithmeticOperation<kFetchFastABSY, true>;
    r[INS_SBC_INDX] = &ArithmeticOperation<kFetchINDX, true>;
    r[INS_SBC_INDY] = &ArithmeticOperation<kFetchINDY, true>;

    // Register Comparison
    r[INS_CMP] = &Register8Compare<&Registers::a, kFetchIM>;
    r[INS_CMP_ZP] = &Register8Compare<&Registers::a, kFetchZP>;
    r[INS_CMP_ZPX] = &Register8Compare<&Registers::a, kFetchZPX>;
    r[INS_CMP_ABS] = &Register8Compare<&Registers::a, kFetchABS>;
    r[INS_CMP_ABSX] = &Register8Compare<&Registers::a, kFetchFastABSX>;
    r[INS_CMP_ABSY] = &Register8Compare<&Registers::a, kFetchFastABSY>;
    r[INS_CMP_INDY] = &Register8Compare<&Registers::a, kFetchINDY>;
    r[INS_CMP_INDX] = &Register8Compare<&Registers::a, kFetchINDX>;
    r[INS_CPX] = &Register8Compare<&Registers::x, kFetchIM>;
    r[INS_CPY] = &Register8Compare<&Registers::y, kFetchIM>;
    r[INS_CPX_ZP] = &Register8Compare<&Registers::x, kFetchZP>;
    r[INS_CPY_ZP] = &Register8Compare<&Registers::y, kFetchZP>;
    r[INS_CPX_ABS] = &Register8Compare<&Registers::x, kFetchABS>;
    r[INS_CPY_ABS] = &Register8Compare<&Registers::y, kFetchABS>;

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
    r[INS_BRK] = &BRK;

    //Logical
    r[INS_AND_IM] = &LogicalOperation<&Operation::AND, kFetchIM>;
    r[INS_AND_ZP] = &LogicalOperation<&Operation::AND, kFetchZP>;
    r[INS_AND_ZPX] = &LogicalOperation<&Operation::AND, kFetchZPX>;
    r[INS_AND_ABS] = &LogicalOperation<&Operation::AND, kFetchABS>;
    r[INS_AND_ABSX] = &LogicalOperation<&Operation::AND, kFetchFastABSX>;
    r[INS_AND_ABSY] = &LogicalOperation<&Operation::AND, kFetchFastABSY>;
    r[INS_AND_INDX] = &LogicalOperation<&Operation::AND, kFetchINDX>;
    r[INS_AND_INDY] = &LogicalOperation<&Operation::AND, kFetchINDY>;
    r[INS_ORA_IM] = &LogicalOperation<&Operation::ORA, kFetchIM>;
    r[INS_ORA_ZP] = &LogicalOperation<&Operation::ORA, kFetchZP>;
    r[INS_ORA_ZPX] = &LogicalOperation<&Operation::ORA, kFetchZPX>;
    r[INS_ORA_ABS] = &LogicalOperation<&Operation::ORA, kFetchABS>;
    r[INS_ORA_ABSX] = &LogicalOperation<&Operation::ORA, kFetchFastABSX>;
    r[INS_ORA_ABSY] = &LogicalOperation<&Operation::ORA, kFetchFastABSY>;
    r[INS_ORA_INDX] = &LogicalOperation<&Operation::ORA, kFetchINDX>;
    r[INS_ORA_INDY] = &LogicalOperation<&Operation::ORA, kFetchINDY>;
    r[INS_EOR_IM] = &LogicalOperation<&Operation::XOR, kFetchIM>;
    r[INS_EOR_ZP] = &LogicalOperation<&Operation::XOR, kFetchZP>;
    r[INS_EOR_ZPX] = &LogicalOperation<&Operation::XOR, kFetchZPX>;
    r[INS_EOR_ABS] = &LogicalOperation<&Operation::XOR, kFetchABS>;
    r[INS_EOR_ABSX] = &LogicalOperation<&Operation::XOR, kFetchFastABSX>;
    r[INS_EOR_ABSY] = &LogicalOperation<&Operation::XOR, kFetchFastABSY>;
    r[INS_EOR_INDX] = &LogicalOperation<&Operation::XOR, kFetchINDX>;
    r[INS_EOR_INDY] = &LogicalOperation<&Operation::XOR, kFetchINDY>;

    //status flag changes
    r[INS_CLC] = &SetFlag<Flags::Carry, false>;
    r[INS_SEC] = &SetFlag<Flags::Carry, true>;
    r[INS_CLD] = &SetFlag<Flags::DecimalMode, false>;
    r[INS_SED] = &SetFlag<Flags::DecimalMode, true>;
    r[INS_CLI] = &SetFlag<Flags::IRQB, false>;
    r[INS_SEI] = &SetFlag<Flags::IRQB, true>;
    r[INS_CLV] = &SetFlag<Flags::Overflow, false>;

    // shifts
    r[INS_ASL] = &Register8Shift<&Registers::a, &Operation::ASL>;
    r[INS_LSR] = &Register8Shift<&Registers::a, &Operation::LSR>;
    r[INS_ROL] = &Register8Shift<&Registers::a, &Operation::ROL>;
    r[INS_ROR] = &Register8Shift<&Registers::a, &Operation::ROR>;

    r[INS_ASL_ZP] = &MemoryShift<&Operation::ASL, kAddressZP>;
    r[INS_ASL_ZPX] = &MemoryShift<&Operation::ASL, kAddressZPX>;
    r[INS_ASL_ABS] = &MemoryShift<&Operation::ASL, kAddressABS>;
    r[INS_ASL_ABSX] = &MemoryShift<&Operation::ASL, kAddressABSX>;
    r[INS_LSR_ZP] = &MemoryShift<&Operation::LSR, kAddressZP>;
    r[INS_LSR_ZPX] = &MemoryShift<&Operation::LSR, kAddressZPX>;
    r[INS_LSR_ABS] = &MemoryShift<&Operation::LSR, kAddressABS>;
    r[INS_LSR_ABSX] = &MemoryShift<&Operation::LSR, kAddressABSX>;
    r[INS_ROL_ZP] = &MemoryShift<&Operation::ROL, kAddressZP>;
    r[INS_ROL_ZPX] = &MemoryShift<&Operation::ROL, kAddressZPX>;
    r[INS_ROL_ABS] = &MemoryShift<&Operation::ROL, kAddressABS>;
    r[INS_ROL_ABSX] = &MemoryShift<&Operation::ROL, kAddressABSX>;
    r[INS_ROR_ZP] = &MemoryShift<&Operation::ROR, kAddressZP>;
    r[INS_ROR_ZPX] = &MemoryShift<&Operation::ROR, kAddressZPX>;
    r[INS_ROR_ABS] = &MemoryShift<&Operation::ROR, kAddressABS>;
    r[INS_ROR_ABSX] = &MemoryShift<&Operation::ROR, kAddressABSX>;

    // stack
    r[INS_TSX] = &Register8Transfer<&Registers::stack_pointer, &Registers::x>;
    r[INS_TXS] = &Register8Transfer<&Registers::x, &Registers::stack_pointer, false>;

    r[INS_PHA] = &StackPush<&Registers::a>;
    r[INS_PLA] = &StackPull<&Registers::a>;
    r[INS_PHP] = &PushFlags;
    r[INS_PLP] = &PullFlags;

    //BIT
    r[INS_BIT_ZP] = &BitOperation<&Operation::AND, kFetchZP>;
    r[INS_BIT_ABS] = &BitOperation<&Operation::AND, kFetchABS>;

    //misc
    r[INS_NOP] = &NOP;

    if (instruction_set == InstructionSet::NMOS6502Emu) {
        r[INS_HLT] = &HLT;
    }

    return r;
}

} // namespace

//-----------------------------------------------------------------------------

std::string Registers::DumpFlags() const {
    std::string r;
    r += fmt::format("{:02x}[", flags);
    r += TestFlag(Flags::Negative) ? "N" : "-";
    r += TestFlag(Flags::Overflow) ? "V" : "-";
    r += TestFlag(Flags::NotUsed) ? "?" : "-";
    r += TestFlag(Flags::Brk) ? "B" : "-";
    r += TestFlag(Flags::DecimalMode) ? "D" : "-";
    r += TestFlag(Flags::IRQB) ? "I" : "-";
    r += TestFlag(Flags::Zero) ? "Z" : "-";
    r += TestFlag(Flags::Carry) ? "C" : "-";
    r += "]";
    return r;
}

std::string Registers::Dump() const {
    std::string r;
    r += fmt::format("pc:{:04x}", program_counter);
    r += fmt::format(" s:{:02x}", stack_pointer);
    r += fmt::format(" a:{:02x}", a);
    r += fmt::format(" x:{:02x}", x);
    r += fmt::format(" y:{:02x}", y);
    r += " f:";
    r += DumpFlags();
    return r;
}

void Registers::Reset() {
    program_counter = 0;
    a = x = y = 0;
    stack_pointer = 0;
    flags = 0;
}

//-----------------------------------------------------------------------------

Cpu::Cpu(InstructionSet instruction_set) : instruction_handlers(&GetInstructionHandlerArray(instruction_set)) {
}

const InstructionHandlerArray &Cpu::GetInstructionHandlerArray(InstructionSet instruction_set) {
    switch (instruction_set) {
    case InstructionSet::NMOS6502: {
        static const InstructionHandlerArray array = GenInstructionHandlerArray(instruction_set);
        return array;
    }
    case InstructionSet::NMOS6502Emu: {
        static const InstructionHandlerArray array = GenInstructionHandlerArray(instruction_set);
        return array;
    }
    case InstructionSet::Unknown:
        break;
    }
    throw std::runtime_error(fmt::format("Invalid instruction set: {}", static_cast<int>(instruction_set)));
}

void Cpu::Reset() {
    reg.Reset();
}

void Cpu::Execute() {
    for (;;) {
        ExecuteNextInstruction();
    }
}

void Cpu::ExecuteWithTimeout(std::chrono::microseconds timeout) {
    auto deadline = std::chrono::steady_clock::now() + timeout;
    while (deadline > std::chrono::steady_clock::now()) {
        ExecuteNextInstruction();
    }
}

void Cpu::ExecuteUntil(uint64_t cycle) {
    while (clock->CurrentCycle() < cycle) {
        ExecuteNextInstruction();
    }
}

void Cpu::ExecuteNextInstruction() {
    auto opcode = instructions::FetchNextByte(this);
    auto handler = (*instruction_handlers)[opcode];
    if (handler == nullptr) {
        throw std::runtime_error(fmt::format("Invalid opcode {:02x} at address {:04x}", opcode, reg.program_counter));
    }
    handler(this);
}

} // namespace emu::emu6502::cpu