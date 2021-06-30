#pragma once

#include "emu_tty/tty_symbol_factory.hpp"
#include "emu_core/text_utils.hpp"
#include "emu_tty/tty_device.hpp"
#include <cstdint>
#include <fmt/format.h>

namespace emu::tty {

SymbolDefVector
TtyDeviceSymbolFactory::GetSymbols(const MemoryConfigEntry &entry,
                                   const MemoryConfigEntry::MappedDevice &md) {

    auto base = entry.offset;
    auto upper_name = ToUpper(entry.name);

    SymbolDefVector r;

    auto emit = [&](auto name, auto value) {
        r.emplace_back(
            SymbolDefinition{.name = fmt::format("TTY_{}_{}", upper_name, name),
                             .value = GetSymbolAddress(value)});
    };

    emit("BASE_ADDRESS", base);
    emit("REGISTER_CONTROL", base + static_cast<uint8_t>(Register::kControl));
    emit("REGISTER_IN_COUNT", base + static_cast<uint8_t>(Register::kInSize));
    emit("REGISTER_OUT_COUNT", base + static_cast<uint8_t>(Register::kOutSize));
    emit("REGISTER_FIFO", base + static_cast<uint8_t>(Register::kFifo));

    return r;
}

} // namespace emu::tty