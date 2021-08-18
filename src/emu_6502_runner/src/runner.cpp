#include "runner.hpp"
#include "emu_6502/cpu/verbose_debugger.hpp"
#include "emu_core/clock_steady.hpp"
#include "emu_core/memory/memory_block.hpp"
#include "emu_core/string_file.hpp"

namespace emu::runner {

void Runner::Setup(const ExecArguments &exec_args) {
    result_verbose = exec_args.GetVerboseStream(Verbose::Result);
    InitCpu(exec_args);
    InitMemory(exec_args);
}

int Runner::Start() {
    auto start = std::chrono::steady_clock::now();
    int code = 0;
    try {
        clock->Reset();
        cpu->Execute();
    } catch (const emu6502::cpu::ExecutionHalted &e) {
        code = e.halt_code;
        if (result_verbose != nullptr) {
            (*result_verbose) << "HALT code: " << std::hex << code << "\n";
        }
    } catch (const std::exception &e) {
        std::cerr << "Run error: " << e.what() << "\n";
        code = -1;
    }
    if (result_verbose != nullptr) {
        auto end = std::chrono::steady_clock::now();
        auto delta = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        auto seconds = delta.count() / 1.0e6;
        (*result_verbose) << fmt::format("Took {:.6f} seconds\n", seconds);
        (*result_verbose) << fmt::format(
            "Cpu cycles: {} ({:.3f} Hz) Lost: {}\n", clock->CurrentCycle(),
            static_cast<double>(clock->CurrentCycle()) / seconds, clock->LostCycles());
    }

    return code;
}

void Runner::InitMemory(const ExecArguments &exec_args) {
    for (auto &dev : exec_args.package->LoadMemoryConfig().entries) {
        auto [device_ptr, size] = std::visit(
            [&](auto &item) { return CreateMemoryDevice(exec_args, dev.name, item); },
            dev.entry_variant);
        if (device_ptr != nullptr) {
            memory->MapArea(static_cast<uint16_t>(dev.offset),
                            static_cast<uint16_t>(size), device_ptr.get());
            mapped_devices.emplace_back(std::move(device_ptr));
        }
    }
}

void Runner::InitCpu(const ExecArguments &exec_args) {
    if (exec_args.cpu_options.frequency == 0) {
        clock = std::make_unique<ClockSimple>();
    } else {
        clock = std::make_unique<ClockSteady>(exec_args.cpu_options.frequency,
                                              exec_args.GetVerboseStream(Verbose::Clock));
    }

    memory = std::make_unique<memory::MemoryMapper16>(
        clock.get(), false, exec_args.GetVerboseStream(Verbose::MemoryMapper));

    auto debug_stream = exec_args.GetVerboseStream(Verbose::Cpu);
    if (debug_stream != nullptr) {
        debugger = std::make_unique<emu6502::cpu::VerboseDebugger>(
            exec_args.cpu_options.instruction_set, memory.get(), clock.get(),
            debug_stream);
    }

    cpu = std::make_unique<emu6502::cpu::Cpu>(clock.get(), memory.get(), debug_stream,
                                              exec_args.cpu_options.instruction_set,
                                              debugger.get());
}

Runner::MappedDevice
Runner::CreateMemoryDevice(const ExecArguments &opts, std::string name,
                           const MemoryConfigEntry::MappedDevice &md) {
    auto device = device_factory->CreateDevice(name, md, clock.get(),
                                               opts.GetVerboseStream(Verbose::Device));
    devices.emplace_back(device);
    return {device->GetMemory(), device->GetMemorySize()};
}

Runner::MappedDevice Runner::CreateMemoryDevice(const ExecArguments &opts,
                                                std::string name,
                                                const MemoryConfigEntry::RamArea &ra) {
    auto mode = ra.writable ? MemoryMode::kReadWrite : MemoryMode::kReadOnly;
    std::vector<uint8_t> bytes;
    if (ra.image.has_value()) {
        bytes = opts.package->LoadFile(ra.image->file, ra.image->offset, ra.size);
    }
    const auto size = bytes.size();
    return {
        std::make_shared<memory::MemoryBlock16>(clock.get(), std::move(bytes), mode,
                                                opts.GetVerboseStream(Verbose::Memory)),
        size,
    };
}

} // namespace emu::runner
