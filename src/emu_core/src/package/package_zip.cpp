#include "emu_core/package/package_zip.hpp"
#include <libzippp.h>

using namespace libzippp;

namespace emu::package {

struct ZipPackage::Impl {
    Impl(const std::string &container_path) : archive(container_path) {
        archive.open(ZipArchive::OpenMode::ReadOnly);
    }
    ~Impl() { //
        archive.close();
    }

    ZipArchive archive;
    std::vector<std::unique_ptr<ByteVector>> stored_data;
};

ZipPackage::ZipPackage(std::string container_path)
    : impl(std::make_unique<Impl>(container_path)) {
}

ZipPackage::~ZipPackage() = default;

MemoryConfig ZipPackage::LoadMemoryConfig() const {
    auto entry = impl->archive.getEntry(kMemoryMetafileName);
    if (entry.isNull()) {
        throw std::runtime_error("Failed to find memory config in package");
    }
    auto config_string = entry.readAsText();
    return LoadMemoryConfigurationFromString(config_string);
}

ByteVector ZipPackage::LoadFile(const std::string &file_name,
                                std::optional<size_t> offset,
                                std::optional<size_t> length) const {
    auto entry = impl->archive.getEntry(file_name);
    if (entry.isNull()) {
        throw std::runtime_error(
            fmt::format("Failed to find '{}' in package", file_name));
    }
    auto data = entry.readAsText();
    auto beg = data.begin() + offset.value_or(0);
    auto to_read = static_cast<std::streamsize>(
        std::min(data.size() - offset.value_or(0), length.value_or(data.size())));
    if (beg + to_read >= data.end()) {
        to_read = data.end() - beg; //trim if overflow
    }
    return ByteVector{beg, beg + to_read};
}

} // namespace emu::package