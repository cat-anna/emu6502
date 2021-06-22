#pragma once

#include "emu_6502/instruction_set.hpp"
#include <emu_core/stream_container.hpp>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace emu::runner {

struct ExecArguments {

    struct CpuOptions {
        uint64_t frequency = 0;
        emu6502::InstructionSet instruction_set = emu6502::InstructionSet::NMOS6502Emu;
    };

    struct MemoryArea {
        uint16_t offset = 0;

        struct MemoryBlock {
            std::vector<uint8_t> bytes{};
            bool rw = true;
        };

        std::variant<MemoryBlock> block;
    };

    using MemoryOptions = std::vector<MemoryArea>;

    bool verbose = false;

    CpuOptions cpu_options;
    MemoryOptions memory_options;

    StreamContainer streams;
};

ExecArguments ParseComandline(int argc, char **argv);

} // namespace emu::runner
