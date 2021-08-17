#pragma once

#include "emu_core/memory_configuration_file.hpp"
#include "emu_core/stream_container.hpp"
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

namespace emu::packager {

struct ExecArguments {
    std::ostream *verbose_stream = nullptr;
    MemoryConfig memory_options;
    StreamContainer streams;
    std::string output_path;
};

ExecArguments ParseComandline(int argc, char **argv);

} // namespace emu::packager
