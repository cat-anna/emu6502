#include "emu_core/package/package_builder_zip.hpp"
#include "emu_core/package/package_zip.hpp"
#include <filesystem>
#include <libzippp.h>

using namespace libzippp;

namespace emu::package {

struct ZipPackageBuilder::Impl {
    Impl(const std::string &output_path) : archive(output_path) {
        if (std::filesystem::exists(output_path)) {
            std::filesystem::remove(output_path);
        }
        archive.open(ZipArchive::OpenMode::Write);
    }
    ~Impl() { //
        archive.close();
    }

    ZipArchive archive;
    std::vector<std::unique_ptr<ByteVector>> stored_data;
};

ZipPackageBuilder::ZipPackageBuilder(const std::string &output_path)
    : impl(std::make_unique<Impl>(output_path)) {
}

ZipPackageBuilder::~ZipPackageBuilder() = default;

void ZipPackageBuilder::SetMemoryConfig(const MemoryConfig &config) const {
    auto str_config = StoreMemoryConfigurationToString(config);
    auto byte_config = std::make_unique<ByteVector>(str_config.begin(), str_config.end());
    impl->archive.addData(kMemoryMetafileName, byte_config->data(), byte_config->size());
    impl->stored_data.emplace_back(std::move(byte_config));
}

void ZipPackageBuilder::AddFile(const ByteVector &data,
                                const std::string &file_name) const {
    auto byte_config = std::make_unique<ByteVector>(data);
    impl->archive.addData(file_name, byte_config->data(), byte_config->size());
    impl->stored_data.emplace_back(std::move(byte_config));
}

} // namespace emu::package
