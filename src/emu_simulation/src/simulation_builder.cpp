#include "emu_core/simulation/simulation_builder.hpp"
#include "emu_6502/cpu/verbose_debugger.hpp"
#include "emu_core/clock_steady.hpp"
#include "emu_core/memory/memory_block.hpp"
#include "emu_core/string_file.hpp"

namespace emu {

struct BuilderState {
    std::shared_ptr<DeviceFactory> device_factory;
    SimulationBuildVerboseConfig verbose;
    package::IPackage *package = nullptr;

    std::unique_ptr<Clock> clock;
    std::unique_ptr<memory::MemoryMapper16> memory;
    std::unique_ptr<emu6502::cpu::Cpu> cpu;
    std::unique_ptr<emu6502::cpu::Debugger> debugger;
    std::vector<std::shared_ptr<Device>> devices;
    std::vector<std::shared_ptr<Memory16>> mapped_devices;

    void InitCpu(const SimulationBuildCpuConfig &cpu_config) {
        if (cpu_config.frequency == 0) {
            clock = std::make_unique<ClockSimple>();
        } else {
            clock = std::make_unique<ClockSteady>(cpu_config.frequency, verbose.clock);
        }

        memory = std::make_unique<memory::MemoryMapper16>(clock.get(), false,
                                                          verbose.memory_mapper);

        if (verbose.cpu != nullptr) {
            debugger = std::make_unique<emu6502::cpu::VerboseDebugger>( //
                cpu_config.instruction_set,                             //
                memory.get(),                                           //
                clock.get(),                                            //
                verbose.cpu                                             //
            );
        }

        cpu = std::make_unique<emu6502::cpu::Cpu>( //
            clock.get(),                           //
            memory.get(),                          //
            verbose.cpu,                           //
            cpu_config.instruction_set,            //
            debugger.get()                         //
        );
    }

    void InitMemory() {
        for (auto &dev : package->LoadMemoryConfig().entries) {
            auto [device_ptr, size] =
                std::visit([&](auto &item) { return CreateMemoryDevice(dev.name, item); },
                           dev.entry_variant);
            if (device_ptr != nullptr) {
                memory->MapArea(static_cast<uint16_t>(dev.offset),
                                static_cast<uint16_t>(size), device_ptr.get());
                mapped_devices.emplace_back(std::move(device_ptr));
            }
        }
    }

    using MappedDevice = std::tuple<std::shared_ptr<Memory16>, size_t>;

    MappedDevice CreateMemoryDevice(std::string name,
                                    const MemoryConfigEntry::MappedDevice &md) {
        auto device = device_factory->CreateDevice(name, md, clock.get(), verbose.device);
        devices.emplace_back(device);
        return {device->GetMemory(), device->GetMemorySize()};
    }

    MappedDevice CreateMemoryDevice(std::string name,
                                    const MemoryConfigEntry::RamArea &ra) {
        auto mode = ra.writable ? MemoryMode::kReadWrite : MemoryMode::kReadOnly;
        std::vector<uint8_t> bytes;
        if (ra.image.has_value()) {
            bytes = package->LoadFile(ra.image->file, ra.image->offset, ra.size);
        }
        const auto size = bytes.size();
        return {
            std::make_shared<memory::MemoryBlock16>(clock.get(), std::move(bytes), mode,
                                                    verbose.memory),
            size,
        };
    }
};

std::unique_ptr<EmuSimulation>
BuildEmuSimulation(std::shared_ptr<DeviceFactory> device_factory,
                   package::IPackage *package, const SimulationBuildCpuConfig &cpu_config,
                   const SimulationBuildVerboseConfig &vc) {
    BuilderState state;
    state.verbose = vc;
    state.package = package;
    state.device_factory = device_factory;

    state.InitCpu(cpu_config);
    state.InitMemory();

    return std::make_unique<EmuSimulation>( //
        std::move(state.clock),             //
        std::move(state.memory),            //
        std::move(state.cpu),               //
        std::move(state.debugger),          //
        std::move(state.devices),           //
        std::move(state.mapped_devices)     //
    );
}

} // namespace emu
