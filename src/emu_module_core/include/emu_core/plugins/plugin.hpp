#pragma once

#include "emu_core/symbol_factory.hpp"
#include <boost/config.hpp>
#include <cstdint>
#include <memory>
#include <vector>

namespace emu::plugins {

#define EMU_DEFINE_SYMBOL_FACTORY(CLASS, NAME)                                           \
    BOOST_SYMBOL_EXPORT std::shared_ptr<emu::SymbolFactory>                              \
        get_symbol_factory_##NAME() {                                                    \
        return std::make_shared<CLASS>();                                                \
    }

using GetSymbolFactory_t = std::shared_ptr<emu::SymbolFactory>();
constexpr auto kGetSymbolFactoryNameFmt = "get_symbol_factory_{}";

} // namespace emu::plugins
