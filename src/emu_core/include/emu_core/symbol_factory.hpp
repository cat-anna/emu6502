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
SymbolAddress GetSymbolAddress(uint32_t v);

struct SymbolDefVectorBuilder {
    SymbolDefVector entries;

    const std::string device_name;
    const std::string class_name;

    SymbolDefVectorBuilder(const std::string &device_name, const std::string &class_name);

    template <typename V, typename O = int>
    void EmitSymbol(const std::string &name, V base, O offset = 0) {
        entries.emplace_back(SymbolDefinition{
            .name = fmt::format("{}_{}_{}", class_name, device_name, name),
            .value = GetSymbolAddress(base + static_cast<V>(offset)),
            .segment = Segment::AbsoluteAddress,
        });
    }
    template <typename V, typename O = int>
    void EmitAlias(const std::string &name, V base, O offset = 0) {
        entries.emplace_back(SymbolDefinition{
            .name = fmt::format("{}_{}_{}", class_name, device_name, name),
            .value = GetSymbolAddress(base + static_cast<V>(offset)),
            .segment = std::nullopt,
        });
    }
};

struct SymbolFactory {
    virtual ~SymbolFactory() = default;

    virtual SymbolDefVector
    GetSymbols(const MemoryConfigEntry &entry,
               const MemoryConfigEntry::MappedDevice &md) const = 0;

    virtual SymbolDefVector GetSymbols(const MemoryConfig &memory_config) const;
};

} // namespace emu
