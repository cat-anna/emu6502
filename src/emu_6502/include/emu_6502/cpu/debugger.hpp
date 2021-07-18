#pragma once

#include "cpu.hpp"
#include "emu_core/memory.hpp"
#include "registers.hpp"
#include <array>
#include <chrono>
#include <cstdint>
#include <string>

namespace emu::emu6502::cpu {

struct Debugger {
    virtual ~Debugger() = default;

    virtual void OnNextInstruction(const Registers &regs) = 0;

    static Debugger *GetVerboseDebugger(InstructionSet instruction_set, Memory16 *memory,
                                        std::ostream *verbose_stream);
};

} // namespace emu::emu6502::cpu
