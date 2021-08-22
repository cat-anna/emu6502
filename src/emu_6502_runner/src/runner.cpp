#include "runner.hpp"
#include "emu_6502/cpu/verbose_debugger.hpp"
#include "emu_core/clock_steady.hpp"
#include "emu_core/memory/memory_block.hpp"
#include "emu_core/simulation/simulation_builder.hpp"
#include "emu_core/string_file.hpp"

namespace emu::runner {

void Runner::Setup(const ExecArguments &exec_args) {
    result_verbose = exec_args.GetVerboseStream(Verbose::Result);

    auto vc = SimulationBuildVerboseConfig{
        .memory = exec_args.GetVerboseStream(Verbose::Memory),
        .memory_mapper = exec_args.GetVerboseStream(Verbose::MemoryMapper),
        .device = exec_args.GetVerboseStream(Verbose::Device),
        .cpu = exec_args.GetVerboseStream(Verbose::Cpu),
        .clock = exec_args.GetVerboseStream(Verbose::Clock),
    };

    auto cpu = SimulationBuildCpuConfig{
        .frequency = exec_args.cpu_options.frequency,
        .instruction_set = exec_args.cpu_options.instruction_set,
    };

    simulation = BuildEmuSimulation(device_factory, exec_args.package.get(), cpu, vc);
}

int Runner::Start() {
    std::optional<EmuSimulation::Result> result;
    try {
        result = simulation->Run();
    } catch (const EmuSimulation::SimulationFailedException &e) {
        if (result_verbose != nullptr) {
            (*result_verbose) << "FATAL: " << e.what() << "\n";
        }
        result = e.GetResult();
    } catch (const std::exception &e) {
        if (result_verbose != nullptr) {
            (*result_verbose) << "FATAL: " << e.what() << "\n";
        }
    }

    if (!result.has_value()) {
        return -1;
    }

    const auto &r = *result;
    if (result_verbose != nullptr) {
        std::string halt_code = "-";
        if (r.halt_code.has_value()) {
            halt_code = std::to_string(r.halt_code.value_or(0));
        }
        (*result_verbose) << fmt::format("Halt code {}\n", halt_code);
        (*result_verbose) << fmt::format("Took {:.6f} seconds\n", r.duration);
        (*result_verbose) << fmt::format("Cpu cycles: {} ({:.3f} Hz)\n", r.cpu_cycles,
                                         static_cast<double>(r.cpu_cycles) / r.duration);
    }

    return r.halt_code.value_or(0);
}

} // namespace emu::runner
