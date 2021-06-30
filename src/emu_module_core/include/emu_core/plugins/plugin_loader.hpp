#pragma once

#include "emu_core/symbol_factory.hpp"
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string_view>
#include <vector>

namespace emu::plugins {

struct PluginLoader {
    virtual ~PluginLoader() = default;

    virtual std::shared_ptr<SymbolFactory> GetSymbolFactory() = 0;
    // virtual std::shared_ptr<DeviceFactory> GetSymbolFactoryInstance() = 0;

    static std::shared_ptr<PluginLoader> CreateDynamic(std::filesystem::path _module_dir);
};

} // namespace emu::plugins
