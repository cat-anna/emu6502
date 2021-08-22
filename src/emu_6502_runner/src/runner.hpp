#pragma once

#include "args.hpp"
#include "emu_6502/cpu/cpu.hpp"
#include "emu_6502/cpu/debugger.hpp"
#include "emu_core/clock.hpp"
#include "emu_core/device_factory.hpp"
#include "emu_core/memory/memory_mapper.hpp"
#include "emu_core/memory_configuration_file.hpp"
#include "emu_core/simulation/simulation.hpp"
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

    std::unique_ptr<EmuSimulation> simulation;
};

} // namespace emu::runner
