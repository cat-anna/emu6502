#pragma once

#include "args.hpp"
#include "emu_core/clock.hpp"
#include "emu_core/device_factory.hpp"
#include "emu_core/memory_configuration_file.hpp"
#include "emu_core/package/package_builder.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace emu::packager {

struct Runner {
    Runner() {}
    int Pack(const ExecArguments &exec_args);

protected:
    std::ostream *verbose_stream = nullptr;
    StreamContainer streams;

    std::unique_ptr<package::IPackageBuilder> package_builder;

    void HandleEntry(MemoryConfigEntry &entry, MemoryConfigEntry::RamArea &ra);
    void HandleEntry(MemoryConfigEntry &entry, MemoryConfigEntry::MappedDevice &md);
};

} // namespace emu::packager
