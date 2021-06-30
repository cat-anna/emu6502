#pragma once

#include "emu_core/plugins/dynamic_plugin_loader.hpp"
#include "emu_core/text_utils.hpp"
#include <boost/dll/import_mangled.hpp>
#include <cstdint>
#include <fmt/format.h>
#include <memory>
#include <string_view>
#include <vector>

namespace emu::plugins {

std::shared_ptr<PluginLoader>
PluginLoader::CreateDynamic(std::filesystem::path _module_dir) {
    return std::make_shared<DynamicPluginLoader>(std::move(_module_dir));
}

std::shared_ptr<SymbolFactory> DynamicPluginLoader::GetSymbolFactory() {
    return std::static_pointer_cast<SymbolFactory>(shared_from_this());
}

SymbolDefVector
DynamicPluginLoader::GetSymbols(const MemoryConfigEntry &entry,
                                const MemoryConfigEntry::MappedDevice &md) {

    if (auto it = symbol_factories.find(md.module_name); it != symbol_factories.end()) {
        return it->second->GetSymbols(entry, md);
    } else {
        try {
            const auto method_name = fmt::format(kGetSymbolFactoryNameFmt, md.class_name);
            auto mod = LoadOrGetModule(md.module_name);
            auto factory_getter =
                dll::experimental::import_mangled<GetSymbolFactory_t>(*mod, method_name);

            auto factory = factory_getter();
            symbol_factories[md.module_name] = factory;
            return factory->GetSymbols(entry, md);
        } catch (const std::exception &e) {
            throw std::runtime_error(fmt::format("Failed to get symbols from {}.{}: {}",
                                                 md.module_name, md.class_name,
                                                 e.what()));
        }
    }
}

SharedSmartModule DynamicPluginLoader::LoadOrGetModule(const std::string &name) {
    if (auto it = modules.find(name); it != modules.end()) {
        return it->second;
    }

    try {
        auto path = dll::fs::path(module_dir) / fmt::format("emu.module.{}", name);
        path = dll::shared_library::decorate(path);
        auto mod = std::make_shared<SharedSmartModule::element_type>(path);
        modules[name] = mod;
        return mod;
    } catch (const std::exception &e) {
        throw std::runtime_error(fmt::format("Failed to import {}: {}", name, e.what()));
    }
}

} // namespace emu::plugins
