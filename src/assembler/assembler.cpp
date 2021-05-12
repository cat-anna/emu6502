#include "assember.hpp"
#include "program_compiler.hpp"
#include <fstream>
#include <sstream>
#include <string_view>

namespace emu6502::assembler {

namespace {} // namespace

std::unique_ptr<Program> CompileString(const std::string &text) {
    std::istringstream ss(text);
    // ss.exceptions(std::ifstream::failbit);
    return CompileStream(ss);
}

std::unique_ptr<Program> CompileFile(const std::string &file_name) {
    std::ifstream f(file_name, std::ios::in);
    f.exceptions(std::ifstream::failbit);
    return CompileStream(f);
}

std::unique_ptr<Program> CompileStream(std::istream &input) {
    std::unique_ptr<Program> program = std::make_unique<Program>();
    ProgramCompiler compiler;

    while (!input.eof()) {
        std::string raw_line;
        std::getline(input, raw_line);

        std::string_view line = raw_line;
        if (!line.empty()) {
            compiler.NextLine(*program, line);
        }
    }

    return program;
}

} // namespace emu6502::assembler
