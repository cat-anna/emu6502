#pragma once

#include "emu_6502/instruction_set.hpp"
#include "emu_core/program.hpp"
#include "tokenizer.hpp"
#include <functional>
#include <iostream>
#include <memory>
#include <string>

namespace emu::emu6502::assembler {

struct CompilationContext;

struct InstructionParsingInfo {
    std::unordered_map<AddressMode, OpcodeInfo> variants;
};

class Compiler6502 {
public:
    Compiler6502(InstructionSet cpu_instruction_set = InstructionSet::Default,
                 bool verbose = true);

    std::unique_ptr<Program> Compile(Tokenizer &tokenizer);
    std::unique_ptr<Program> Compile(std::istream &stream, const std::string &name);

    std::unique_ptr<Program> CompileString(std::string text);
    std::unique_ptr<Program> CompileFile(const std::string &file);

private:
    std::unordered_map<std::string_view, InstructionParsingInfo> instruction_set;
    bool verbose = true;

    void ProcessLine(CompilationContext &context, LineTokenizer &line);
};

std::unique_ptr<Program>
CompileString(std::string text,
              InstructionSet cpu_instruction_set = InstructionSet::Default);
std::unique_ptr<Program>
CompileFile(const std::string &file,
            InstructionSet cpu_instruction_set = InstructionSet::Default);

} // namespace emu::emu6502::assembler
