#pragma once

#include "memory_configuration_file.hpp"
#include "program.hpp"
#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

namespace emu {
struct SymbolDefinition {
    std::string name;
    SymbolAddress value;
    std::optional<Segment> segment = std::nullopt;
};

using SymbolDefVector = std::vector<SymbolDefinition>;

struct SymbolFactory {
    virtual ~SymbolFactory() = default;

    virtual SymbolDefVector GetSymbols(const MemoryConfigEntry &entry,
                                       const MemoryConfigEntry::MappedDevice &md) = 0;

    virtual SymbolDefVector GetSymbols(const MemoryConfig &memory_config);
};

SymbolAddress GetSymbolAddress(uint32_t v);

} // namespace emu
