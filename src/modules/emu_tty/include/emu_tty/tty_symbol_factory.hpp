#pragma once

#include "emu_core/symbol_factory.hpp"
#include <cstdint>
#include <memory>

namespace emu::tty {

struct TtyDeviceSymbolFactory : public SymbolFactory {
    TtyDeviceSymbolFactory() = default;
    virtual ~TtyDeviceSymbolFactory() = default;

    SymbolDefVector GetSymbols(const MemoryConfigEntry &entry,
                               const MemoryConfigEntry::MappedDevice &md) override;
};

} // namespace emu::tty