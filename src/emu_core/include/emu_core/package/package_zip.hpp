#pragma once

#include "emu_core/memory_configuration_file.hpp"
#include "package.hpp"
#include <cstdint>
#include <fmt/format.h>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace emu::package {

constexpr auto kMemoryMetafileName = ".memory.yaml";

class ZipPackage : public IPackage {
public:
    ~ZipPackage() override = default;
    ZipPackage(std::string container_path);

    MemoryConfig LoadMemoryConfig() const override;
    ByteVector LoadFile(const std::string &file_name,
                        std::optional<size_t> offset = std::nullopt,
                        std::optional<size_t> length = std::nullopt) const override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace emu::package