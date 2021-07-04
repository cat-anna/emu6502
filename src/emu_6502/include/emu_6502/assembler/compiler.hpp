#pragma once

#include "emu_6502/instruction_set.hpp"
#include "emu_core/program.hpp"
#include "emu_core/symbol_factory.hpp"
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
    ~Compiler6502();

    void Compile(Tokenizer &tokenizer);
    void Compile(std::istream &stream, const std::string &name);
    void CompileString(std::string text, const std::string &name = "{string}");
    void CompileFile(const std::string &file);

    void AddDefinitions(const SymbolDefVector &symbols);
    std::unique_ptr<Program> GetProgram();

private:
    std::unordered_map<std::string_view, InstructionParsingInfo> instruction_set;
    bool verbose = true;

    std::unique_ptr<Program> program;
    std::unique_ptr<CompilationContext> context;

    void Start();

    void ProcessLine(LineTokenizer &line);
    bool TryDefinition(const Token &first_token, LineTokenizer &line);
};

std::unique_ptr<Program>
CompileString(std::string text,
              InstructionSet cpu_instruction_set = InstructionSet::Default);

std::unique_ptr<Program>
CompileFile(const std::string &file,
            InstructionSet cpu_instruction_set = InstructionSet::Default);

std::string GenerateSymbolDump(Program &progam);

} // namespace emu::emu6502::assembler
