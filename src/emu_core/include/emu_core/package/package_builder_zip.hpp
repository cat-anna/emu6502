#pragma once

#include "package_builder.hpp"

namespace emu::package {

using ByteVector = std::vector<uint8_t>;

class ZipPackageBuilder : public IPackageBuilder {
public:
    ~ZipPackageBuilder() override;
    ZipPackageBuilder(const std::string &output_path);

    void SetMemoryConfig(const MemoryConfig &config) const override;
    void AddFile(const ByteVector &data, const std::string &file_name) const override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace emu::package
