#pragma once

#include "emu_core/memory_configuration_file.hpp"
#include <cstdint>
#include <fmt/format.h>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace emu::package {

using ByteVector = std::vector<uint8_t>;

class IPackage {
public:
    virtual ~IPackage() = default;

    virtual MemoryConfig LoadMemoryConfig() const = 0;

    virtual ByteVector LoadFile(const std::string &file_name,
                                std::optional<size_t> offset = std::nullopt,
                                std::optional<size_t> length = std::nullopt) const = 0;
};

} // namespace emu::package
