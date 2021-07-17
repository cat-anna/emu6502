#pragma once

#include "emu_6502/instruction_set.hpp"
#include "emu_core/memory_configuration_file.hpp"
#include "emu_core/stream_container.hpp"
#include <set>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace emu::runner {

enum class Verbose {
    Result,
    Cpu,
    Device,
    Memory,
    MemoryMapper,
};

struct ExecArguments {

    struct CpuOptions {
        uint64_t frequency = 0;
        emu6502::InstructionSet instruction_set = emu6502::InstructionSet::NMOS6502Emu;
    };

    std::set<Verbose> verbose;
    std::ostream *verbose_stream = &std::cout;
    std::ostream *GetVerboseStream(Verbose v) const;

    CpuOptions cpu_options;
    MemoryConfig memory_options;

    StreamContainer streams;
};

ExecArguments ParseComandline(int argc, char **argv);

} // namespace emu::runner
