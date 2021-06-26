#pragma once

#include "emu_6502/cpu/cpu.hpp"
#include "emu_core/memory.hpp"
#include "memory_addressing.hpp"
#include <emu_core/clock.hpp>

namespace emu::emu6502::cpu::instructions {

using Flags = Registers::Flags;

using MemAddrFunc = MemPtr (*)(Cpu *cpu);
using MemReadFunc = uint8_t (*)(Cpu *cpu);
using Reg8Ptr = Reg8(Registers::*);

//-----------------------------------------------------------------------------

template <typename OffType>
bool IsAcrossPage(MemPtr base, OffType inc) {
    auto page0 = (base >> 8) & 0xFF;
    auto r = base + inc;
    auto page1 = ((r >> 8) & 0xFF);
    return page0 != page1;
}

//-----------------------------------------------------------------------------

void NOP(Cpu *cpu) {
    cpu->WaitForNextCycle();
}

template <MemReadFunc read_func>
void HLT(Cpu *cpu) {
    auto value = read_func(cpu);
    throw ExecutionHalted(cpu->reg, value);
}

template <uint8_t opcode>
void InvalidOpcode(Cpu *cpu) {
    throw InvalidOpcodeException(cpu->reg, opcode);
}

//-----------------------------------------------------------------------------

template <Reg8Ptr target, MemReadFunc read_func>
void Register8Load(Cpu *cpu) {
    auto value = read_func(cpu);
    cpu->reg.*target = value;
    cpu->reg.SetNegativeZeroFlag(value);
}

template <Reg8Ptr target, MemAddrFunc addr_func>
void Register8Store(Cpu *cpu) {
    auto value = cpu->reg.*target;
    cpu->memory->Store(addr_func(cpu), value);
}

template <Reg8Ptr source, Reg8Ptr target, bool set_flags = true>
void Register8Transfer(Cpu *cpu) {
    auto value = cpu->reg.*source;
    if constexpr (set_flags) {
        cpu->reg.SetNegativeZeroFlag(value);
    }
    cpu->reg.*target = value;
    cpu->WaitForNextCycle();
}

template <Reg8Ptr source, int8_t direction>
void Register8Increment(Cpu *cpu) {
    auto value = cpu->reg.*source;
    if constexpr (direction > 0) {
        ++value;
    } else {
        --value;
    }
    cpu->reg.SetNegativeZeroFlag(value);
    cpu->reg.*source = value;
    cpu->WaitForNextCycle();
}

//-----------------------------------------------------------------------------

template <MemAddrFunc addr_func, int8_t direction>
void MemoryIncrement(Cpu *cpu) {
    auto addr = addr_func(cpu);
    auto value = cpu->memory->Load(addr);
    if constexpr (direction > 0) {
        ++value;
    } else {
        --value;
    }
    cpu->reg.SetNegativeZeroFlag(value);
    cpu->WaitForNextCycle();
    cpu->memory->Store(addr, value);
}

//-----------------------------------------------------------------------------

template <Reg8Ptr source, MemReadFunc read_func>
void Register8Compare(Cpu *cpu) {
    auto src = cpu->reg.*source;
    auto operand = read_func(cpu);
    cpu->reg.SetNegativeZeroFlag(src - operand);
    cpu->reg.SetFlag(Flags::Carry, src >= operand);
}

//-----------------------------------------------------------------------------

using LogicFunc = uint8_t (*)(uint8_t, uint8_t);
using ShiftFunc = std::tuple<uint8_t, bool> (*)(uint8_t, bool);

constexpr Reg8 kMSB = 0x80;
constexpr Reg8 kLSB = 0x01;

struct Operation {
    static uint8_t AND(uint8_t a, uint8_t b) { return a & b; }
    static uint8_t ORA(uint8_t a, uint8_t b) { return a | b; }
    static uint8_t XOR(uint8_t a, uint8_t b) { return a ^ b; }

    static std::tuple<uint8_t, bool> ASL(uint8_t v, bool carry) {
        return std::make_tuple(static_cast<uint8_t>(v << 1), (v & kMSB) != 0);
    }
    static std::tuple<uint8_t, bool> LSR(uint8_t v, bool carry) {
        return std::make_tuple(static_cast<uint8_t>(v >> 1), (v & kLSB) != 0);
    }
    static std::tuple<uint8_t, bool> ROL(uint8_t v, bool carry) {
        return std::make_tuple(static_cast<uint8_t>((v << 1) | (carry ? kLSB : 0)),
                               (v & kMSB) != 0);
    }
    static std::tuple<uint8_t, bool> ROR(uint8_t v, bool carry) {
        return std::make_tuple(static_cast<uint8_t>((v >> 1) | (carry ? kMSB : 0)),
                               (v & kLSB) != 0);
    }
};

template <LogicFunc op, MemReadFunc read_func>
void LogicalOperation(Cpu *cpu) {
    auto operand = read_func(cpu);
    auto result = op(cpu->reg.a, operand);
    cpu->reg.SetNegativeZeroFlag(result);
    cpu->reg.a = result;
}

template <LogicFunc op, MemReadFunc read_func>
void BitOperation(Cpu *cpu) {
    auto operand = read_func(cpu);
    auto result = op(cpu->reg.a, operand);
    cpu->reg.SetNegativeZeroFlag(result);
    cpu->reg.SetFlag(Registers::Flags::Overflow, (result & 0x40) > 0);
}

template <Reg8Ptr source, ShiftFunc op>
void Register8Shift(Cpu *cpu) {
    auto operand = cpu->reg.*source;
    cpu->WaitForNextCycle();
    auto [result, new_carry] = op(operand, cpu->reg.TestFlag(Flags::Carry));
    cpu->reg.SetNegativeZeroFlag(result);
    cpu->reg.SetFlag(Flags::Carry, new_carry);
    cpu->reg.*source = result;
}

template <ShiftFunc op, MemAddrFunc addr_func>
void MemoryShift(Cpu *cpu) {
    auto addr = addr_func(cpu);
    auto operand = cpu->memory->Load(addr);
    cpu->WaitForNextCycle();
    auto [result, new_carry] = op(operand, cpu->reg.TestFlag(Flags::Carry));
    cpu->reg.SetNegativeZeroFlag(result);
    cpu->reg.SetFlag(Flags::Carry, new_carry);
    cpu->memory->Store(addr, result);
}

//-----------------------------------------------------------------------------

template <MemReadFunc read_func, bool subtract = false>
void ArithmeticOperation(Cpu *cpu) {
    auto operand = read_func(cpu);
    if (cpu->reg.TestFlag(Flags::DecimalMode)) {
        throw std::runtime_error("Decimal mode is not implemented yet");
    } else {
        const bool SignBitsAreEqual = ((cpu->reg.a ^ operand) & kNegativeBit) == 0;

        uint16_t result = cpu->reg.a;
        result += cpu->reg.CarryValue();
        if constexpr (subtract) {
            result -= operand;
        } else {
            result += operand;
        }

        cpu->reg.a = result & 0xFF;
        cpu->reg.SetNegativeZeroFlag(cpu->reg.a);
        cpu->reg.SetFlag(Flags::Carry, result > 0xFF);
        cpu->reg.SetFlag(Flags::Overflow, SignBitsAreEqual && ((cpu->reg.a ^ operand) &
                                                               kNegativeBit) != 0);
    }
}

//-----------------------------------------------------------------------------

template <Flags flag, bool state>
void SetFlag(Cpu *cpu) {
    cpu->WaitForNextCycle();
    cpu->reg.SetFlag(flag, state);
}

//-----------------------------------------------------------------------------

void StackPushByte(Cpu *cpu, uint8_t v, bool reuse_cycle = false) {
    cpu->memory->Store(cpu->reg.StackPointerMemoryAddress(), v);
    if (!reuse_cycle) {
        cpu->WaitForNextCycle();
    }
    cpu->reg.stack_pointer--;
}

uint8_t StackPullByte(Cpu *cpu, bool reuse_cycle = false) {
    cpu->reg.stack_pointer++;
    auto operand = cpu->memory->Load(cpu->reg.StackPointerMemoryAddress());
    if (!reuse_cycle) {
        cpu->WaitForNextCycle();
    }
    return operand;
}

//-----------------------------------------------------------------------------

template <Reg8Ptr source>
void StackPush(Cpu *cpu) {
    StackPushByte(cpu, cpu->reg.*source);
}

template <Reg8Ptr source>
void StackPull(Cpu *cpu) {
    auto operand = StackPullByte(cpu);
    cpu->WaitForNextCycle();
    cpu->reg.SetNegativeZeroFlag(operand);
    cpu->reg.*source = operand;
}

void PushFlags(Cpu *cpu) {
    uint8_t operand = cpu->reg.flags | static_cast<uint8_t>(Flags::Brk) |
                      static_cast<uint8_t>(Flags::NotUsed);
    StackPushByte(cpu, operand);
}

void PullFlags(Cpu *cpu) {
    auto operand = cpu->memory->Load(cpu->reg.StackPointerMemoryAddress());
    cpu->WaitForNextCycle();
    cpu->reg.flags = operand;
    cpu->reg.SetFlag(Flags::Brk, false);
    cpu->reg.SetFlag(Flags::NotUsed, false);
    cpu->WaitForNextCycle();
    cpu->reg.stack_pointer++;
}

//-----------------------------------------------------------------------------

template <Registers::Flags flag, bool state>
void Branch(Cpu *cpu) {
    auto offset_address = static_cast<int8_t>(FetchNextByte(cpu));
    if (cpu->reg.TestFlag(flag) == state) {
        cpu->WaitForNextCycle();
        if (IsAcrossPage(cpu->reg.program_counter, offset_address)) {
            cpu->WaitForNextCycle();
        }
        cpu->reg.program_counter += offset_address;
    }
}

void JumpABS(Cpu *cpu) {
    auto addr = GetAbsoluteAddress(cpu);
    cpu->reg.program_counter = addr;
}

void JumpIND(Cpu *cpu) {
    auto addr = GetAbsoluteAddress(cpu);
    MemPtr fetched_address = cpu->memory->Load(addr);
    addr = (addr & 0xFF00) | ((addr + 1) & 0xFF);
    fetched_address |= cpu->memory->Load(addr) << 8;
    cpu->reg.program_counter = fetched_address;
}

void JSR(Cpu *cpu) { //
    auto addr = GetAbsoluteAddress(cpu);
    cpu->reg.program_counter -= 1;
    StackPushByte(cpu, cpu->reg.program_counter >> 8);
    StackPushByte(cpu, cpu->reg.program_counter & 0xff, true);
    cpu->reg.program_counter = addr;
}

void RTS(Cpu *cpu) { //
    uint16_t low = StackPullByte(cpu);
    uint16_t hi = StackPullByte(cpu);
    cpu->WaitForNextCycle();
    cpu->reg.program_counter = (hi << 8 | low) + 1;
}

void RTI(Cpu *cpu) {
    uint16_t low = StackPullByte(cpu);
    uint16_t hi = StackPullByte(cpu, true);
    cpu->reg.program_counter = (hi << 8 | low);
    cpu->reg.flags = StackPullByte(cpu);
}

void BRK(Cpu *cpu) {
    for (int i = 0; i < 6; ++i) {
        cpu->WaitForNextCycle();
    }

    ++cpu->reg.program_counter;
    cpu->reg.SetFlag(Flags::Brk, true);
    //TODO: do correct implementation
}

//-----------------------------------------------------------------------------

template <MemAddrFunc addr_func>
uint8_t FetchMemory(Cpu *cpu) {
    return cpu->memory->Load(addr_func(cpu));
}

constexpr auto kFetchIM = &FetchNextByte;
constexpr auto kFetchZP = &FetchMemory<kAddressZP>;
constexpr auto kFetchZPX = &FetchMemory<kAddressZPX>;
constexpr auto kFetchZPY = &FetchMemory<kAddressZPY>;
constexpr auto kFetchABS = &FetchMemory<kAddressABS>;

constexpr auto kFetchABSX = &FetchMemory<kAddressABSX>;
constexpr auto kFetchABSY = &FetchMemory<kAddressABSY>;
constexpr auto kFetchFastABSX = &FetchMemory<kAddressFastABSX>;
constexpr auto kFetchFastABSY = &FetchMemory<kAddressFastABSY>;

constexpr auto kFetchINDX = &FetchMemory<kAddressINDX>;
constexpr auto kFetchINDY = &FetchMemory<kAddressINDY>;

constexpr auto kFetchAcc = &FetchAccumulator;

} // namespace emu::emu6502::cpu::instructions
