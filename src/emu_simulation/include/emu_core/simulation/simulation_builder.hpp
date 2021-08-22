#pragma once

#include "emu_6502/cpu/cpu.hpp"
#include "emu_6502/cpu/debugger.hpp"
#include "emu_core/clock.hpp"
#include "emu_core/device_factory.hpp"
#include "emu_core/memory/memory_mapper.hpp"
#include "emu_core/memory_configuration_file.hpp"
#include "emu_core/package/package.hpp"
#include "simulation.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace emu {

struct SimulationBuildVerboseConfig {
    std::ostream *memory = nullptr;
    std::ostream *memory_mapper = nullptr;
    std::ostream *device = nullptr;
    std::ostream *cpu = nullptr;
    std::ostream *clock = nullptr;

    static SimulationBuildVerboseConfig SingleSteam(std::ostream *o,
                                                    bool with_mapper = true) {
        return SimulationBuildVerboseConfig{
            .memory = o,
            .memory_mapper = (with_mapper ? o : nullptr),
            .device = o,
            .cpu = o,
            .clock = o,
        };
    }
    static SimulationBuildVerboseConfig Stdout() {
        return SingleSteam(&std::cout, false);
    }
};

struct SimulationBuildCpuConfig {
    uint64_t frequency;
    emu6502::InstructionSet instruction_set;
};

std::unique_ptr<EmuSimulation>
BuildEmuSimulation(std::shared_ptr<DeviceFactory> device_factory,
                   package::IPackage *package, const SimulationBuildCpuConfig &cpu_config,
                   const SimulationBuildVerboseConfig &vc = {});

} // namespace emu
