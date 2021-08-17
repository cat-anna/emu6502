#include "runner.hpp"
#include "emu_core/clock_steady.hpp"
#include "emu_core/memory/memory_block.hpp"
#include "emu_core/package/package_builder_zip.hpp"
#include "emu_core/string_file.hpp"

namespace emu::packager {

int Runner::Pack(const ExecArguments &exec_args) {
    verbose_stream = exec_args.verbose_stream;

    int code = 0;

    package_builder = std::make_unique<package::ZipPackageBuilder>(exec_args.output_path);

    auto memory_options = exec_args.memory_options;

    for (auto &entry : memory_options.entries) {
        std::visit([this, &entry](auto &value) { HandleEntry(entry, value); },
                   entry.entry_variant);
    }

    package_builder->SetMemoryConfig(memory_options);
    package_builder.reset();

    return code;
}

void Runner::HandleEntry(MemoryConfigEntry &entry, MemoryConfigEntry::RamArea &ra) {
    if (!ra.image.has_value()) {
        return;
    }

    auto &image = *ra.image;
    auto input = streams.OpenBinaryInput(image.file);
    auto file_size = static_cast<uint64_t>(std::filesystem::file_size(image.file));
    if (image.offset.has_value()) {
        auto offset = image.offset.value();
        input->seekg(offset, std::ios::beg);
        if (offset > file_size) {
            file_size = 0;
        } else {
            file_size -= offset;
        }
    }

    if (ra.size.has_value()) {
        file_size = std::min(file_size, ra.size.value());
    }
    ra.size = file_size;

    auto file_name = fmt::format("{:04x}_{:04x}", entry.offset, file_size);

    if (!entry.name.empty()) {
        file_name += "_" + entry.name;
    }

    file_name += ".bin";

    package::ByteVector data(file_size, '\0');
    input->read(reinterpret_cast<char *>(&data[0]), file_size);

    image = MemoryConfigEntry::RamArea::Image{
        .file = file_name,
        .offset = 0,
    };
    package_builder->AddFile(data, file_name);
}

void Runner::HandleEntry(MemoryConfigEntry &entry, MemoryConfigEntry::MappedDevice &md) {
    // nothing
}

} // namespace emu::packager
