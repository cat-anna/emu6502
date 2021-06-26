#include "emu_core/symbol_factory.hpp"

namespace emu {

std::vector<SymbolFactory::SymbolDetails>
SymbolFactory::GetSymbols(const MemoryConfig &memory_config,
                          SymbolFactory *symbol_factory) {
    std::vector<SymbolDetails> r;

    for (auto &entry : memory_config.entries) {
        auto v = symbol_factory->GetSymbols(entry);
        r.insert(r.end(), v.begin(), v.end());
    }

    return r;
}

} // namespace emu
