#pragma once

#include "cpu.hpp"
#include "debugger.hpp"
#include "emu_6502/instruction_set.hpp"
#include "emu_core/memory.hpp"
#include <array>
#include <chrono>
#include <cstdint>
#include <string>

namespace emu::emu6502::cpu {

struct VerboseDebugger : public Debugger {
    VerboseDebugger(InstructionSet instruction_set, Memory16 *memory,
                    std::ostream *verbose_stream);
    void OnNextInstruction(const Registers &regs) override;

private:
    Memory16 *const memory;
    std::ostream *const verbose_stream;

    std::array<const OpcodeInfo *, 256> known_opcodes;
};

} // namespace emu::emu6502::cpu
