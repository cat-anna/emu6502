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

constexpr auto kEmuImageExtension = ".emu_image";

using ByteVector = std::vector<uint8_t>;

class IPackageBuilder {
public:
    virtual ~IPackageBuilder() = default;

    virtual void SetMemoryConfig(const MemoryConfig &config) const = 0;
    virtual void AddFile(const ByteVector &data, const std::string &file_name) const = 0;
};

} // namespace emu::package
