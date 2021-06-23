#include "emu_core/memory_configuration_file.hpp"
#include <fmt/format.h>
#include <iostream>
#include <map>
#include <unordered_map>
#pragma warning(push)
#pragma warning(disable : 4251 4996)
#include "yaml-cpp/yaml.h"
#pragma warning(pop)

namespace YAML {

namespace {

template <typename T>
std::optional<T> ReadOptional(const std::string &name, const Node &node) {
    if (auto n = node[name]; n) {
        return n.as<T>();
    }
    return std::nullopt;
}

std::string ReadString(const std::string &name, const Node &node, bool optional = false) {
    if (auto n = node[name]; n) {
        auto r = n.as<std::string>();
        if (r.empty() && !optional) {
            throw std::runtime_error(fmt::format("'{}' is expected to contain text"));
        }
        return r;
    } else {
        if (!optional) {
            throw std::runtime_error(fmt::format("'{}' is expected to contain text"));
        }
        return "";
    }
}

} // namespace

template <>
struct convert<emu::MemoryConfigEntry::RamArea::Image> {
    static bool decode(const Node &node, emu::MemoryConfigEntry::RamArea::Image &rhs) {
        if (!node.IsMap()) {
            return false;
        }

        rhs.file = ReadString("file", node);
        rhs.offset = ReadOptional<uint32_t>("offset", node);
        return true;
    }
};

template <>
struct convert<emu::MemoryConfigEntry::RamArea> {
    static bool decode(const Node &node, emu::MemoryConfigEntry::RamArea &rhs) {
        if (!node.IsMap()) {
            return false;
        }

        if (node["ram"]) {
            rhs.writable = true;
        } else if (node["rom"]) {
            rhs.writable = false;
        } else {
            throw std::runtime_error("Unknown memory config entry RamArea mode");
        }

        rhs.image = ReadOptional<emu::MemoryConfigEntry::RamArea::Image>("image", node);
        rhs.size = ReadOptional<uint32_t>("size", node);
        return true;
    }
};

template <>
struct convert<emu::MemoryConfigEntry::ValueVariant> {
    static bool decode(const Node &node, emu::MemoryConfigEntry::ValueVariant &rhs) {
        if (node.IsNull()) {
            rhs = std::monostate{};
            return true;
        }

        if (!node.IsScalar()) {
            return false;
        }

        auto v = node.as<std::string>();
        if (v == "true") {
            rhs = true;
            return true;
        }
        if (v == "false") {
            rhs = false;
            return true;
        }

        try {
            size_t pos = 0;
            if (auto r = std::stoll(v, &pos, 0); pos == v.size()) {
                rhs = r;
                return true;
            }
        } catch (...) {
            //ignore
        }
        try {
            size_t pos = 0;
            if (auto r = std::stod(v, &pos); pos == v.size()) {
                rhs = r;
                return true;
            }
        } catch (...) {
            //ignore
        }

        rhs = v;
        return true;
    }
};

template <>
struct convert<emu::MemoryConfigEntry::MappedDevice> {
    static bool decode(const Node &node, emu::MemoryConfigEntry::MappedDevice &rhs) {
        if (!node.IsMap()) {
            return false;
        }

        rhs.class_name = ReadString("class", node);

        if (auto config = node["config"]; config) {
            rhs.config = config.as<decltype(rhs.config)>();
        }

        // std::unordered_map<std::string, ValueVariant> config;

        return true;
    }
};

template <>
struct convert<emu::MemoryConfigEntry> {
    static bool decode(const Node &node, emu::MemoryConfigEntry &rhs) {
        if (!node.IsMap()) {
            return false;
        }

        rhs.offset = node["offset"].as<uint32_t>();
        rhs.name = ReadString("name", node, true);

        if (node["ram"] || node["rom"]) {
            rhs.entry_variant = node.as<emu::MemoryConfigEntry::RamArea>();
        } else if (node["device"]) {
            rhs.entry_variant = node.as<emu::MemoryConfigEntry::MappedDevice>();
        } else {
            throw std::runtime_error("Unknown memory config entry");
        }

        return true;
    }
};

} // namespace YAML

namespace emu {

namespace {

MemoryConfig Load(YAML::Node config) {
    return MemoryConfig{
        .entries = config["memory"].as<std::vector<MemoryConfigEntry>>(),
    };
}

} // namespace

MemoryConfig LoadMemoryConfigurationFromFile(const std::string &file_name) {
    return Load(YAML::LoadFile(file_name));
}

MemoryConfig LoadMemoryConfigurationFromString(const std::string &text) {
    return Load(YAML::Load(text));
}

// std::string StoreMemoryConfigurationToString(const MemoryConfig &config) {
//     return "";
// }

} // namespace emu
