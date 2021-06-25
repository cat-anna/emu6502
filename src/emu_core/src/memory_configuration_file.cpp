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
    static Node encode(const emu::MemoryConfigEntry::RamArea::Image &rhs) {
        Node node;
        node["file"] = rhs.file;
        if (rhs.offset.has_value()) {
            node["offset"] = rhs.offset.value();
        }
        return node;
    }
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
    static Node encode(const emu::MemoryConfigEntry::ValueVariant &rhs) {
        return std::visit([](auto &item) -> Node { return StoreNode(item); }, rhs);
    }
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
        node["class"] = rhs.class_name;
        node["config"] = rhs.config;
        node["device"] = {};
        return node;
    }
    static bool decode(const Node &node, emu::MemoryConfigEntry::MappedDevice &rhs) {
        if (!node.IsMap()) {
            return false;
        }

        rhs.class_name = ReadString("class", node);

        if (auto config = node["config"]; config) {
            rhs.config = config.as<decltype(rhs.config)>();
        }

        return true;
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

std::vector<MemoryConfigEntry> LoadMemoryConfigEntryVector(const YAML::Node &node,
                                                           FileSearch *searcher) {
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
            auto sub = LoadMemoryConfigEntryVector(loaded_yaml["memory"], searcher);
            rhs.insert(rhs.end(), sub.begin(), sub.end());
        } else {
            rhs.emplace_back(i.as<MemoryConfigEntry>());
        }
    }

    return rhs;
}

MemoryConfig Load(YAML::Node config, FileSearch *searcher) {
    return MemoryConfig{
        .entries = LoadMemoryConfigEntryVector(config["memory"], searcher),
    };
}

} // namespace

MemoryConfig LoadMemoryConfigurationFromFile(const std::string &file_name,
                                             FileSearch *searcher) {
    auto yaml = YAML::LoadFile(file_name);
    auto s = searcher->PrependPath(file_name);
    return MemoryConfig{
        .entries = LoadMemoryConfigEntryVector(yaml["memory"], s.get()),
    };
}

MemoryConfig LoadMemoryConfigurationFromString(const std::string &text,
                                               FileSearch *searcher) {
    return Load(YAML::Load(text), searcher);
}

std::string StoreMemoryConfigurationToString(const MemoryConfig &config) {
    YAML::Node node;
    node["memory"] = config.entries;
    std::stringstream ss;
    ss << node << "\n";
    return ss.str();
}

} // namespace emu
