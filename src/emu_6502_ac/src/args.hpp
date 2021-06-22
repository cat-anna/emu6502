#pragma once

#include "emu_6502/instruction_set.hpp"
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
        std::string input_name;
        std::istream *input = nullptr;
    };

    struct Output {
        std::ostream *binary_output = nullptr;
        std::ostream *hex_dump = nullptr;
    };

    bool verbose = false;

    Cpu cpu_options;
    Input input_options;
    Output output_options;

    StreamContainer streams;
};

ExecArguments ParseComandline(int argc, char **argv);

} // namespace emu::emu6502::assembler
