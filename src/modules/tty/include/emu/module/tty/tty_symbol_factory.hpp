#pragma once

#include "emu_core/symbol_factory.hpp"
#include <cstdint>
#include <memory>

namespace emu::module::tty {

struct TtyDeviceSymbolFactory : public SymbolFactory {
    TtyDeviceSymbolFactory() = default;
    ~TtyDeviceSymbolFactory() override = default;

    SymbolDefVector GetSymbols(const MemoryConfigEntry &entry,
                               const MemoryConfigEntry::MappedDevice &md) const override;
};

} // namespace emu::module::tty