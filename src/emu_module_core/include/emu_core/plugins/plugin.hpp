#pragma once

#include "emu_core/device_factory.hpp"
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

//-----------------------------------------------------------------------------

using GetDeviceFactory_t = std::shared_ptr<emu::DeviceFactory>();
constexpr auto kGetDeviceFactoryNameFmt = "get_device_factory_{}";

#define EMU_DEFINE_DEVICE_FACTORY(CLASS, NAME)                                           \
    BOOST_SYMBOL_EXPORT std::shared_ptr<emu::DeviceFactory>                              \
        get_device_factory_##NAME() {                                                    \
        return std::make_shared<CLASS>();                                                \
    }

//-----------------------------------------------------------------------------

#define EMU_DEFINE_FACTORIES(DEVICE, SYMBOL, NAME)                                       \
    EMU_DEFINE_DEVICE_FACTORY(DEVICE, NAME)                                              \
    EMU_DEFINE_SYMBOL_FACTORY(SYMBOL, NAME)

} // namespace emu::plugins
