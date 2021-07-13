#include "runner.hpp"
#include "emu_core/clock_steady.hpp"
#include "emu_core/memory/memory_block.hpp"
#include "emu_core/string_file.hpp"
// #include <emu_core/build_config.hpp>
// #include "emu_core/memory/memory_sparse.hpp"
// #include <emu_core/program.hpp>

namespace emu::runner {

void Runner::Setup(const ExecArguments &exec_args) {
    verbose = exec_args.verbose;
    InitCpu(exec_args.cpu_options);
    InitMemory(exec_args.memory_options);
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

void Runner::InitMemory(const MemoryConfig &opts) {
    for (auto &dev : opts.entries) {
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

void Runner::InitCpu(const ExecArguments::CpuOptions &opts) {
    if (opts.frequency == 0) {
        clock = std::make_unique<ClockSimple>();
    } else {
        clock = std::make_unique<ClockSteady>(opts.frequency);
    }

    memory = std::make_unique<memory::MemoryMapper16>(clock.get(), false, verbose);

    cpu = std::make_unique<emu6502::cpu::Cpu>(clock.get(), memory.get(), verbose,
                                              opts.instruction_set);
}

Runner::MappedDevice
Runner::CreateMemoryDevice(std::string name, const MemoryConfigEntry::MappedDevice &md) {
    auto device = device_factory->CreateDevice(name, md, clock.get());
    devices.emplace_back(device);
    return {device->GetMemory(), device->GetMemorySize()};
}

Runner::MappedDevice Runner::CreateMemoryDevice(std::string name,
                                                const MemoryConfigEntry::RamArea &ra) {
    auto mode = ra.writable ? MemoryMode::kReadWrite : MemoryMode::kReadOnly;
    std::vector<uint8_t> bytes;
    if (ra.image.has_value()) {
        auto data = load_file<std::vector<uint8_t>>(ra.image->file);
        if (auto offset = ra.image->offset.value_or(0); offset == 0) {
            bytes.swap(data);
        } else {
            bytes.insert(bytes.end(), data.begin() + offset, data.end());
        }
    }
    bytes.resize(ra.size.value_or(bytes.size()), 0);
    auto size = bytes.size();
    return {
        std::make_shared<memory::MemoryBlock16>(clock.get(), std::move(bytes), mode,
                                                verbose),
        size,
    };
}

} // namespace emu::runner
