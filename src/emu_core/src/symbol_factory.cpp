#include "emu_core/symbol_factory.hpp"
#include "emu_core/program.hpp"
#include "emu_core/text_utils.hpp"
#include <fmt/format.h>

namespace emu {

namespace {

SymbolDefVector GetSymbolDefs(const SymbolFactory *symbol_factory,
                              const MemoryConfigEntry &entry,
                              const MemoryConfigEntry::RamArea &ra) {
    SymbolDefVector r;
    r.emplace_back(SymbolDefinition{
        .name = ToUpper(entry.name) + "_ADDRESS",
        .value = GetSymbolAddress(entry.offset),
        .segment = Segment::AbsoluteAddress,
    });

    if (ra.size.has_value()) {
        r.emplace_back(SymbolDefinition{
            .name = ToUpper(entry.name) + "_SIZE",
            .value = GetSymbolAddress(ra.size.value()),
            .segment = std::nullopt,
        });
    }
    return r;
}

SymbolDefVector GetSymbolDefs(const SymbolFactory *symbol_factory,
                              const MemoryConfigEntry &entry,
                              const MemoryConfigEntry::MappedDevice &md) {
    return symbol_factory->GetSymbols(entry, md);
}

} // namespace

SymbolDefVectorBuilder::SymbolDefVectorBuilder(const std::string &device_name,
                                               const std::string &class_name)
    : device_name(ToUpper(device_name)), class_name(ToUpper(class_name)) {
}

SymbolDefVector SymbolFactory::GetSymbols(const MemoryConfig &memory_config) const {
    SymbolDefVector r;

    for (auto &entry : memory_config.entries) {
        if (entry.name.empty()) {
            continue;
        }

        auto v = std::visit(
            [&](auto &item) -> SymbolDefVector {
                return GetSymbolDefs(this, entry, item);
            },
            entry.entry_variant);
        r.insert(r.end(), v.begin(), v.end());
    }

    return r;
}

SymbolAddress GetSymbolAddress(uint32_t v) {
    if (v <= std::numeric_limits<uint8_t>::max()) {
        return static_cast<uint8_t>(v);
    }
    if (v <= std::numeric_limits<Address_t>::max()) {
        return static_cast<Address_t>(v);
    }
    throw std::runtime_error(fmt::format("'{}' is too large to fit SymbolAddress", v));
}

} // namespace emu
