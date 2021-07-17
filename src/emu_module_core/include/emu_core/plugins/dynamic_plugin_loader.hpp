#pragma once

#include "plugin.hpp"
#include "plugin_loader.hpp"
#include <boost/dll/import.hpp>
#include <boost/dll/smart_library.hpp>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace emu::plugins {

namespace dll = boost::dll;
using SharedSmartModule = std::shared_ptr<dll::experimental::smart_library>;

struct DynamicPluginLoader : public PluginLoader,
                             public SymbolFactory,
                             public DeviceFactory,
                             public std::enable_shared_from_this<DynamicPluginLoader> {
    DynamicPluginLoader(std::filesystem::path _module_dir) : module_dir(_module_dir) {}
    ~DynamicPluginLoader() override = default;

    //PluginLoader
    std::shared_ptr<SymbolFactory> GetSymbolFactory() override;
    std::shared_ptr<DeviceFactory> GetDeviceFactory() override;

    //SymbolFactory
    SymbolDefVector GetSymbols(const MemoryConfigEntry &entry,
                               const MemoryConfigEntry::MappedDevice &md) override;

    //DeviceFactory
    std::shared_ptr<Device> CreateDevice(const std::string &name,
                                         const MemoryConfigEntry::MappedDevice &md,
                                         Clock *clock,
                                         std::ostream *verbose_output = nullptr) override;

private:
    std::filesystem::path const module_dir;

    std::unordered_map<std::string, SharedSmartModule> modules;
    std::unordered_map<std::string, std::shared_ptr<SymbolFactory>> symbol_factories;
    std::unordered_map<std::string, std::shared_ptr<DeviceFactory>> device_factories;

    SharedSmartModule LoadOrGetModule(const std::string &name);
};

} // namespace emu::plugins
