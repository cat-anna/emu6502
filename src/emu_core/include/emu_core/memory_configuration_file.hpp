#pragma once

#include "file_search.hpp"
#include <cstdint>
#include <fmt/format.h>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace emu {

struct MemoryConfigEntry {
    using ValueVariant = std::variant<std::monostate, std::string, int64_t, bool, double>;

    template <typename T>
    static T GetValueVariant(const ValueVariant &vv, T &&t) {
        if (std::holds_alternative<std::monostate>(vv)) {
            return std::forward<T>(t);
        }
        if (std::holds_alternative<T>(vv)) {
            return std::get<std::decay_t<T>>(vv);
        }

        std::visit(
            [](auto &item) {
                throw std::runtime_error(
                    fmt::format("Failed to convert config paramter from '{}' to '{}'",
                                typeid(item).name(), typeid(T).name()));
            },
            vv);
        abort();
    }

    struct RamArea {
        struct Image {
            std::string file;
            std::optional<uint32_t> offset;
            bool operator==(const Image &o) const = default;
        };

        std::optional<Image> image;
        std::optional<uint32_t> size;
        bool writable;
        bool operator==(const RamArea &o) const = default;
    };

    struct MappedDevice {
        std::string module_name;
        std::string class_name;
        std::map<std::string, ValueVariant> config;
        bool operator==(const MappedDevice &o) const = default;

        template <typename T>
        T GetConfigItem(const std::string &key, T t) const {
            if (auto it = config.find(key); it != config.end()) {
                return GetValueVariant<T>(it->second, std::move(t));
            } else {
                return std::move(t);
            }
        }
    };

    std::string name;
    uint32_t offset;
    std::variant<RamArea, MappedDevice> entry_variant;

    bool operator==(const MemoryConfigEntry &o) const = default;
};

struct MemoryConfig {
    std::vector<MemoryConfigEntry> entries;

    bool operator==(const MemoryConfig &o) const = default;
};

MemoryConfig LoadMemoryConfigurationFromFile(const std::string &file_name,
                                             FileSearch *searcher = nullptr);
MemoryConfig LoadMemoryConfigurationFromString(const std::string &text,
                                               FileSearch *searcher = nullptr);

std::string StoreMemoryConfigurationToString(const MemoryConfig &config);

} // namespace emu
