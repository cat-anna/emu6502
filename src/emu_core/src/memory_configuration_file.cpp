#include "emu_core/memory_configuration_file.hpp"
#include <fmt/format.h>
#include <iostream>
#include <map>
#include <unordered_map>
#include <yaml-cpp/yaml.h>

namespace {

std::string HandleOverride(std::string input, const emu::ConfigOverrides &overrides) {
    if (input.empty()) {
        return {};
    }
    if (input.front() != '$') {
        return input;
    }

    std::string key{input.begin() + 1, input.end()};
    if (auto it = overrides.find(key); it == overrides.end()) {
        return input; //?
        // throw std::runtime_error(fmt::format("Override '{}' is not defined", input));
    } else {
        return it->second;
    }
}

template <typename T>
std::optional<T> ReadOptional(const std::string &name, const YAML::Node &node) {
    if (auto n = node[name]; n) {
        return n.as<T>();
    }
    return std::nullopt;
}

std::string ReadString(const std::string &name, const YAML::Node &node, bool optional,
                       const emu::ConfigOverrides &overrides) {
    if (auto n = node[name]; n) {
        auto r = HandleOverride(n.as<std::string>(), overrides);
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

namespace YAML {

template <>
struct convert<emu::MemoryConfigEntry::RamArea::Image> {
    static Node encode(const emu::MemoryConfigEntry::RamArea::Image &rhs) {
        Node node;
        node["file"] = rhs.file;
        if (rhs.offset.has_value()) {
            node["offset"] = rhs.offset.value();
        }
        return node;
    }
};

template <>
struct convert<emu::MemoryConfigEntry::RamArea> {
    static Node encode(const emu::MemoryConfigEntry::RamArea &rhs) {
        Node node;
        if (rhs.writable) {
            node["ram"] = Node{};
        } else {
            node["rom"] = Node{};
        }
        if (rhs.size.has_value()) {
            node["size"] = rhs.size.value();
        }
        if (rhs.image.has_value()) {
            node["image"] = *rhs.image;
        }
        return node;
    }
};

template <>
struct convert<emu::MemoryConfigEntry::ValueVariant> {
    static Node encode(const emu::MemoryConfigEntry::ValueVariant &rhs) {
        return std::visit([](auto &item) -> Node { return StoreNode(item); }, rhs);
    }

private:
    static Node StoreNode(const std::monostate &) { return Node{}; }
    static Node StoreNode(const std::string &s) { return Node{s}; }
    static Node StoreNode(int64_t i) { return Node{i}; }
    static Node StoreNode(bool b) { return Node{b}; }
    static Node StoreNode(double d) { return Node{d}; }
};

template <>
struct convert<emu::MemoryConfigEntry::MappedDevice> {
    static Node encode(const emu::MemoryConfigEntry::MappedDevice &rhs) {
        Node node;
        node["class"] = rhs.module_name + "." + rhs.class_name;
        node["config"] = rhs.config;
        node["device"] = {};
        return node;
    }
};

template <>
struct convert<emu::MemoryConfigEntry> {
    static Node encode(const emu::MemoryConfigEntry &rhs) {
        auto node = std::visit([&](auto &item) -> Node { return StoreNode(item); },
                               rhs.entry_variant);
        node["offset"] = rhs.offset;
        if (!rhs.name.empty()) {
            node["name"] = rhs.name;
        }
        return node;
    }

private:
    static Node StoreNode(const emu::MemoryConfigEntry::MappedDevice &md) {
        return convert<emu::MemoryConfigEntry::MappedDevice>::encode(md);
    }
    static Node StoreNode(const emu::MemoryConfigEntry::RamArea &ra) {
        return convert<emu::MemoryConfigEntry::RamArea>::encode(ra);
    }
};

} // namespace YAML

namespace emu {

namespace {

MemoryConfigEntry::ValueVariant LoadValueVariant(const YAML::Node &node,
                                                 const ConfigOverrides &overrides) {
    if (node.IsNull()) {
        return std::monostate{};
    }

    if (!node.IsScalar()) {
        return false;
    }

    auto v = HandleOverride(node.as<std::string>(), overrides);
    if (v == "true") {
        return true;
    }
    if (v == "false") {
        return false;
    }

    try {
        size_t pos = 0;
        if (auto r = std::stoll(v, &pos, 0); pos == v.size()) {
            return r;
        }
    } catch (...) {
        //ignore
    }
    try {
        size_t pos = 0;
        if (auto r = std::stod(v, &pos); pos == v.size()) {
            return r;
        }
    } catch (...) {
        //ignore
    }

    return v;
}

std::map<std::string, MemoryConfigEntry::ValueVariant>
LoadValueVariantMap(const YAML::Node &node, const ConfigOverrides &overrides) {
    if (!node.IsMap()) {
        throw std::runtime_error("Unknown memory config entry ValueVariant type");
    }
    std::map<std::string, MemoryConfigEntry::ValueVariant> r;
    for (auto &item : node) {
        r[item.first.as<std::string>()] = LoadValueVariant(item.second, overrides);
    }
    return r;
}

std::optional<MemoryConfigEntry::RamArea::Image>
LoadRamAreaImageEntry(const YAML::Node &node, const ConfigOverrides &overrides) {
    if (!node.IsMap()) {
        return std::nullopt;
    }

    MemoryConfigEntry::RamArea::Image rhs;
    rhs.file = ReadString("file", node, false, overrides);
    rhs.offset = ReadOptional<uint64_t>("offset", node);
    return rhs;
}

MemoryConfigEntry::MappedDevice LoadMappedDeviceEntry(const YAML::Node &node,
                                                      const ConfigOverrides &overrides) {
    MemoryConfigEntry::MappedDevice rhs;
    if (!node.IsMap()) {
        throw std::runtime_error("Unknown memory config entry MappedDevice type");
    }

    auto cl = ReadString("class", node, false, overrides);
    if (auto pos = cl.find('.'); pos == std::string::npos) {
        rhs.module_name = cl;
        rhs.class_name = "default";
    } else {
        rhs.module_name = cl.substr(0, pos);
        rhs.class_name = cl.substr(pos + 1);
    }

    if (auto config = node["config"]; config) {
        rhs.config = LoadValueVariantMap(config, overrides);
    }

    return rhs;
}

MemoryConfigEntry::RamArea LoadRamAreaEntry(const YAML::Node &node,
                                            const ConfigOverrides &overrides) {

    MemoryConfigEntry::RamArea rhs;

    if (!node.IsMap()) {
        throw std::runtime_error("Unknown memory config entry RamArea type");
    }

    if (static_cast<bool>(node["ram"])) {
        rhs.writable = true;
    } else if (static_cast<bool>(node["rom"])) {
        rhs.writable = false;
    } else {
        throw std::runtime_error("Unknown memory config entry RamArea mode");
    }

    if (auto n = node["image"]; n) {
        rhs.image = LoadRamAreaImageEntry(n, overrides);
    }
    rhs.size = ReadOptional<uint64_t>("size", node);

    return rhs;
}

MemoryConfigEntry LoadMemoryConfigEntry(const YAML::Node &node,
                                        const ConfigOverrides &overrides) {

    if (!node.IsMap()) {
        throw std::runtime_error("Invalid memory config entry type");
    }

    MemoryConfigEntry rhs;
    rhs.offset = node["offset"].as<uint64_t>();
    rhs.name = ReadString("name", node, true, overrides);

    if (static_cast<bool>(node["ram"]) || static_cast<bool>(node["rom"])) {
        rhs.entry_variant = LoadRamAreaEntry(node, overrides);
    } else if (static_cast<bool>(node["device"])) {
        rhs.entry_variant = LoadMappedDeviceEntry(node, overrides);
    } else {
        throw std::runtime_error("Unknown memory config entry");
    }

    return rhs;
}

std::vector<MemoryConfigEntry>
LoadMemoryConfigEntryVector(const YAML::Node &node, FileSearch *searcher,
                            const ConfigOverrides &overrides) {
    if (!node.IsSequence()) {
        throw std::runtime_error("Malformed memory list configuration");
    }

    std::vector<MemoryConfigEntry> rhs;

    for (auto i : node) {
        if (auto incl = i["include"]; incl) {
            auto file = incl.as<std::string>();
            if (searcher == nullptr) {
                throw std::runtime_error("Cannot search for mem config include " + file);
            }
            auto input = searcher->OpenFile(file);
            auto loaded_yaml = YAML::Load(*input);
            auto s = searcher->PrependPath(file);
            auto sub =
                LoadMemoryConfigEntryVector(loaded_yaml["memory"], searcher, overrides);
            rhs.insert(rhs.end(), sub.begin(), sub.end());
        } else {
            rhs.emplace_back(LoadMemoryConfigEntry(i, overrides));
        }
    }

    return rhs;
}

MemoryConfig Load(YAML::Node config, FileSearch *searcher,
                  const ConfigOverrides &overrides) {
    return MemoryConfig{
        .entries = LoadMemoryConfigEntryVector(config["memory"], searcher, overrides),
    };
}

} // namespace

MemoryConfig LoadMemoryConfigurationFromFile(const std::string &file_name,
                                             FileSearch *searcher,
                                             const ConfigOverrides &overrides) {
    auto yaml = YAML::LoadFile(file_name);
    auto s = searcher->PrependPath(file_name);
    return MemoryConfig{
        .entries = LoadMemoryConfigEntryVector(yaml["memory"], s.get(), overrides),
    };
}

MemoryConfig LoadMemoryConfigurationFromString(const std::string &text,
                                               FileSearch *searcher,
                                               const ConfigOverrides &overrides) {
    return Load(YAML::Load(text), searcher, overrides);
}

std::string StoreMemoryConfigurationToString(const MemoryConfig &config) {
    YAML::Node node;
    node["memory"] = config.entries;
    std::stringstream ss;
    ss << node << "\n";
    return ss.str();
}

} // namespace emu
