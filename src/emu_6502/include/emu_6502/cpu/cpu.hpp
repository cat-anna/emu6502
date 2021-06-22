#pragma once

#include "emu_6502/instruction_set.hpp"
#include <array>
#include <chrono>
#include <cstdint>
#include <emu_core/clock.hpp>
#include <emu_core/memory.hpp>
#include <string>

namespace emu::emu6502::cpu {

struct Cpu;

using OperandFunctionPtr = void (*)(Cpu *cpu);
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

struct ExecutionHalted : public std::exception {
    ExecutionHalted(Registers regs, Reg8 halt_code)
        : std::exception("Execution halted"), regs(regs), halt_code(halt_code) {}
    const Registers regs;
    const Reg8 halt_code;
};

struct InvalidOpcodeException : public std::exception {
    InvalidOpcodeException(Registers regs, Reg8 opcode)
        : std::exception("Invalid opcode"), regs(regs), opcode(opcode) {}
    const Registers regs;
    const Reg8 opcode;
};

struct Cpu {
    Registers reg;
    Memory16 *const memory;
    const InstructionHandlerArray *instruction_handlers;

    Cpu(Clock *clock, Memory16 *memory, bool verbose, InstructionSet instruction_set = InstructionSet::NMOS6502);

    static const InstructionHandlerArray &GetInstructionHandlerArray(InstructionSet instruction_set);

    void Execute();
    void ExecuteFor(std::chrono::nanoseconds timeout);
    void ExecuteUntil(std::chrono::steady_clock::time_point deadline);
    void ExecuteNextInstruction();

    void Reset();

    void WaitForNextCycle() const;

private:
    Clock *const clock;
    const bool verbose;
};

} // namespace emu::emu6502::cpu
