#pragma once

#include "debugger.hpp"
#include "emu_6502/instruction_set.hpp"
#include "emu_core/memory.hpp"
#include "registers.hpp"

#include <array>
#include <chrono>
#include <cstdint>
#include <emu_core/clock.hpp>
#include <string>

namespace emu::emu6502::cpu {

struct Cpu;

using OperandFunctionPtr = void (*)(Cpu *cpu);
using InstructionHandlerArray = std::array<OperandFunctionPtr, 256>;

struct ExecutionHalted : public std::runtime_error {
    ExecutionHalted(Registers regs, Reg8 halt_code)
        : std::runtime_error("Execution halted"), regs(regs), halt_code(halt_code) {}
    const Registers regs;
    const Reg8 halt_code;
};

struct InvalidOpcodeException : public std::runtime_error {
    InvalidOpcodeException(Registers regs, Reg8 opcode)
        : std::runtime_error("Invalid opcode"), regs(regs), opcode(opcode) {}
    const Registers regs;
    const Reg8 opcode;
};

struct Cpu {
    Registers reg;
    Memory16 *const memory;
    const InstructionHandlerArray *instruction_handlers;

    Cpu(Clock *clock, Memory16 *memory, std::ostream *verbose_stream = nullptr,
        InstructionSet instruction_set = InstructionSet::NMOS6502,
        Debugger *external_debugger = nullptr);

    static const InstructionHandlerArray &
    GetInstructionHandlerArray(InstructionSet instruction_set);

    void Execute();
    void ExecuteFor(std::chrono::nanoseconds timeout);
    void ExecuteUntil(std::chrono::steady_clock::time_point deadline);
    void ExecuteNextInstruction();

    void Reset();

    void WaitForNextCycle() const;

private:
    Clock *const clock;
    std::ostream *const verbose_stream;
    Debugger *const debugger;
};

} // namespace emu::emu6502::cpu
