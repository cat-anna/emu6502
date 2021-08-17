#pragma once

#include "emu_core/file_search.hpp"
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

class FsPackage : public IPackage {
public:
    ~FsPackage() override = default;
    FsPackage(std::string config_file, std::shared_ptr<FileSearch> searcher = nullptr);
    FsPackage(MemoryConfig config, std::shared_ptr<FileSearch> searcher = nullptr);

    MemoryConfig LoadMemoryConfig() const override;
    ByteVector LoadFile(const std::string &file_name,
                        std::optional<size_t> offset = std::nullopt,
                        std::optional<size_t> length = std::nullopt) const override;

private:
    const MemoryConfig config;
    std::shared_ptr<FileSearch> searcher;
};

} // namespace emu::package