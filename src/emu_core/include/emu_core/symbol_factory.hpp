#pragma once

#include "memory_configuration_file.hpp"
#include "program.hpp"
#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

namespace emu {

struct SymbolFactory {
    virtual ~SymbolFactory() = default;

    struct SymbolDetails {
        std::string name;
        SymbolAddress value;
        // Segment segment = Segment::Unknown;
    };

    virtual std::vector<SymbolDetails>
    GetSymbols(const MemoryConfigEntry &config_entry) = 0;

    static std::vector<SymbolDetails> GetSymbols(const MemoryConfig &memory_config,
                                                 SymbolFactory *symbol_factory = nullptr);
};

} // namespace emu
