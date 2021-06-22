#pragma once

#include "args.hpp"
#include <emu_6502/cpu/cpu.hpp>
#include <emu_core/clock.hpp>
#include <emu_core/memory.hpp>
#include <emu_core/memory_mapper.hpp>
#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace emu::runner {

struct Runner {
    Runner() = default;
    Runner &Setup(const ExecArguments &exec_args);
    int Start();

protected:
    std::unique_ptr<Clock> clock;
    std::unique_ptr<MemoryMapper16> memory;
    std::unique_ptr<emu6502::cpu::Cpu> cpu;

    bool verbose = false;

    std::vector<std::unique_ptr<Memory16>> mapped_memory_devices;

    void InitCpu(const ExecArguments::CpuOptions &opts);
    void InitMemory(const ExecArguments::MemoryOptions &opts);

    using MappedDevice = std::tuple<std::unique_ptr<Memory16>, uint16_t>;

    MappedDevice CreateMemoryDevice(const ExecArguments::MemoryArea::MemoryBlock &block);
};

} // namespace emu::runner
