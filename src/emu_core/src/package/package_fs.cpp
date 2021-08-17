#include "emu_core/package/package_fs.hpp"
#include <fstream>

namespace emu::package {

FsPackage::FsPackage(std::string config_file, std::shared_ptr<FileSearch> searcher)
    : FsPackage(LoadMemoryConfigurationFromFile(config_file), std::move(searcher)) {
}

FsPackage::FsPackage(MemoryConfig config, std::shared_ptr<FileSearch> searcher)
    : config(config), searcher(std::move(searcher)) {
}

MemoryConfig FsPackage::LoadMemoryConfig() const {
    return config;
}

ByteVector FsPackage::LoadFile(const std::string &file_name, std::optional<size_t> offset,
                               std::optional<size_t> length) const {
    auto file = searcher->OpenFile(file_name);
    file->seekg(0, std::ios::end);
    auto file_size = static_cast<size_t>(file->tellg());
    file->seekg(offset.value_or(0), std::ios::beg);
    auto to_read = std::min(file_size - offset.value_or(0), length.value_or(file_size));
    ByteVector data;
    data.resize(to_read);
    file->read(reinterpret_cast<char *>(&data[0]), to_read);
    return data;
}

} // namespace emu::package