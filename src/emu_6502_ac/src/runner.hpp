#pragma once

#include "args.hpp"
// #include <emu_core/clock.hpp>
// #include <emu_core/memory.hpp>
#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace emu::emu6502::assembler {

struct Runner {
    Runner() = default;
    int Start(const ExecArguments &exec_args);

protected:
    bool verbose = false;

    // std::unique_ptr<Clock> clock;
    // std::unique_ptr<Memory16> memory;

    // std::vector<std::unique_ptr<Memory16>> mapped_memory_devices;

    // void InitClock(const ExecutionOptions &execution_args);
    // void InitMemory(const MemoryOptions &memory_opts);
    // void InitCpu(const ExecutionOptions &execution_args);
};

} // namespace emu::emu6502::assembler
