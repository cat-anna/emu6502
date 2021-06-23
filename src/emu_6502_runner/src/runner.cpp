#include "runner.hpp"
// #include <emu_core/build_config.hpp>
#include <emu_core/clock_steady.hpp>
#include <emu_core/memory_block.hpp>
// #include <emu_core/memory_sparse.hpp>
// #include <emu_core/program.hpp>

namespace emu::runner {

Runner &Runner::Setup(const ExecArguments &exec_args) {
    verbose = exec_args.verbose;

    InitCpu(exec_args.cpu_options);
    InitMemory(exec_args.memory_options);

    return *this;
}

int Runner::Start() {
    auto start = std::chrono::steady_clock::now();
    int code = 0;
    try {
        clock->Reset();
        cpu->Execute();
    } catch (const emu6502::cpu::ExecutionHalted &e) {
        code = e.halt_code;
        if (verbose) {
            std::cout << "HALT code: " << std::hex << code << "\n";
        }
    } catch (const std::exception &e) {
        std::cerr << "Run error: " << e.what() << "\n";
        code = -1;
    }
    if (verbose) {
        auto end = std::chrono::steady_clock::now();
        auto delta = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << fmt::format("Took {} seconds\n", delta.count() / (1024.0 * 1024.0));
    }
    return code;
}

void Runner::InitMemory(const ExecArguments::MemoryOptions &opts) {
    for (auto &dev : opts) {
        auto [device_ptr, size] = std::visit(
            [this](auto &item) { return CreateMemoryDevice(item); }, dev.block);
        memory->MapArea(dev.offset, size, device_ptr.get());
        mapped_memory_devices.emplace_back(std::move(device_ptr));
    }
}

void Runner::InitCpu(const ExecArguments::CpuOptions &opts) {
    if (opts.frequency == 0) {
        clock = std::make_unique<ClockSimple>();
    } else {
        clock = std::make_unique<ClockSteady>(opts.frequency);
    }

    memory = std::make_unique<MemoryMapper16>(clock.get(), false, verbose);

    cpu = std::make_unique<emu6502::cpu::Cpu>(clock.get(), memory.get(), verbose,
                                              opts.instruction_set);
}

Runner::MappedDevice
Runner::CreateMemoryDevice(const ExecArguments::MemoryArea::MemoryBlock &block) {
    auto mode = block.rw ? MemoryMode::kReadWrite : MemoryMode::kReadOnly;
    return {
        std::make_unique<MemoryBlock16>(clock.get(), block.bytes, mode, verbose),
        static_cast<uint16_t>(block.bytes.size() - 1), //TODO: this is awful
    };
}

} // namespace emu::runner
