#pragma once

#include "instruction_set.hpp"
#include <chrono>
#include <cstdint>
#include <emu_core/clock.hpp>
#include <emu_core/memory.hpp>
#include <string>

namespace emu::cpu6502 {

using Reg8 = uint8_t;
using Reg16 = uint16_t;
using MemPtr = uint16_t;

constexpr MemPtr kIrqVector = 0xFFFE;
constexpr MemPtr kResBVector = 0xFFFC;
constexpr MemPtr kNmibVector = 0xFFFA;

constexpr MemPtr kStackBase = 0x0100;
constexpr Reg8 kNegativeBit = 0x80;

struct Cpu6502;

using OperandFunctionPtr = void (*)(Cpu6502 *cpu);
using InstructionHandlerArray = std::array<OperandFunctionPtr, 256>;

struct Registers {
    Reg16 program_counter;

    Reg8 a, x, y;
    Reg8 stack_pointer;

    Reg8 flags;

    enum class Flags {
        Carry = 0x01,
        Zero = 0x02,
        IRQB = 0x04,
        DecimalMode = 0x08,
        Brk = 0x10,
        NotUsed = 0x20,
        Overflow = 0x40,
        Negative = 0x80,
    };

    void Reset();

    std::string DumpFlags() const;
    std::string Dump() const;

    bool TestFlag(Flags f) const { return (flags & static_cast<Reg8>(f)) == static_cast<Reg8>(f); }
    void SetFlag(Flags f, bool value) {
        if (value) {
            flags |= static_cast<Reg8>(f);
        } else {
            flags &= ~static_cast<Reg8>(f);
        }
    }

    void SetNegativeZeroFlag(uint8_t v) {
        SetFlag(Flags::Zero, v == 0);
        SetFlag(Flags::Negative, (v & kNegativeBit) != 0);
    }
    void SetNegativeFlag(uint8_t v) { SetFlag(Flags::Negative, (v & kNegativeBit) != 0); }

    uint8_t CarryValue() const { return TestFlag(Flags::Carry) ? 1 : 0; }

    MemPtr StackPointerMemoryAddress() const { return kStackBase | stack_pointer; }
};

struct Cpu6502 {
    Registers reg;
    Clock *clock;
    Memory *memory;
    const InstructionHandlerArray &instruction_handlers;

    Cpu6502(InstructionSet instruction_set = InstructionSet::NMOS6502);

    static const InstructionHandlerArray &GetInstructionHandlerArray(InstructionSet instruction_set);

    void Execute();
    void ExecuteWithTimeout(std::chrono::microseconds timeout);
    void ExecuteUntil(uint64_t cycle);
    void ExecuteNextInstruction();

    void Reset();
};

} // namespace emu::cpu6502
