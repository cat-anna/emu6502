#pragma once

#include "args.hpp"
#include "emu_6502/cpu/cpu.hpp"
#include "emu_6502/cpu/debugger.hpp"
#include "emu_core/clock.hpp"
#include "emu_core/device_factory.hpp"
#include "emu_core/memory/memory_mapper.hpp"
#include "emu_core/memory_configuration_file.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace emu::runner {

struct Runner {
    Runner(std::shared_ptr<DeviceFactory> _device_factory)
        : device_factory(_device_factory) {}

    void Setup(const ExecArguments &exec_args);
    int Start();

protected:
    const std::shared_ptr<DeviceFactory> device_factory;
    std::ostream *result_verbose = nullptr;

    std::unique_ptr<Clock> clock;
    std::unique_ptr<memory::MemoryMapper16> memory;
    std::unique_ptr<emu6502::cpu::Cpu> cpu;
    std::unique_ptr<emu6502::cpu::Debugger> debugger;

    std::vector<std::shared_ptr<Device>> devices;
    std::vector<std::shared_ptr<Memory16>> mapped_devices;

    void InitCpu(const ExecArguments &exec_args);
    void InitMemory(const ExecArguments &exec_args);

    using MappedDevice = std::tuple<std::shared_ptr<Memory16>, size_t>;

    MappedDevice CreateMemoryDevice(const ExecArguments &opts, std::string name,
                                    const MemoryConfigEntry::RamArea &ra);
    MappedDevice CreateMemoryDevice(const ExecArguments &opts, std::string name,
                                    const MemoryConfigEntry::MappedDevice &md);
};

} // namespace emu::runner
