#pragma once

#include "emu_6502/instruction_set.hpp"
#include "emu_core/memory_configuration_file.hpp"
#include <emu_core/stream_container.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace emu::emu6502::assembler {

struct ExecArguments {
    struct Cpu {
        InstructionSet instruction_set = InstructionSet::NMOS6502Emu;
    };

    struct Input {
        std::string name;
        std::istream *stream = nullptr;
    };

    struct Output {
        std::ostream *binary_output = nullptr;
        std::ostream *hex_dump = nullptr;
        std::ostream *symbol_dump = nullptr;
    };

    bool verbose = false;

    Cpu cpu_options;
    std::vector<Input> input_options;
    Output output_options;
    MemoryConfig memory_options;

    StreamContainer streams;
};

ExecArguments ParseComandline(int argc, char **argv);

} // namespace emu::emu6502::assembler
