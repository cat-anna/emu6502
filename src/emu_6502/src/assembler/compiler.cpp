#include "emu_6502/assembler/compiler.hpp"
#include "compilation_context.hpp"
#include "emu_6502/assembler/compilation_error.hpp"
#include "emu_core/base16.hpp"
#include "emu_core/container_utils.hpp"
#include <fstream>
#include <sstream>

namespace emu::emu6502::assembler {

std::unique_ptr<Program> CompileString(std::string text,
                                       InstructionSet cpu_instruction_set) {
    Compiler6502 c{cpu_instruction_set};
    c.CompileString(std::move(text));
    return c.GetProgram();
}

std::unique_ptr<Program> CompileFile(const std::string &file,
                                     InstructionSet cpu_instruction_set) {
    Compiler6502 c{cpu_instruction_set};
    c.CompileFile(file);
    return c.GetProgram();
}

std::string GenerateSymbolDump(Program &program) {
    std::stringstream ss;
    ss << ";Aliases:\n";
    for (auto &name : SortedKeys(program.aliases)) {
        auto ptr = program.aliases.at(name);
        ss << fmt::format("{} = {}\n", name, FormatHex(ptr->value, ""));
    }
    ss << ";Symbols:\n";
    for (auto &name : SortedKeys(program.symbols)) {
        auto ptr = program.symbols.at(name);
        auto hex = FormatHex(ToBytes(ptr->offset, std::nullopt), "");
        ss << fmt::format(".symbol {}, {}, {}\n", name, hex,
                          (ptr->imported ? "true" : "false"));
    }
    return ss.str();
}

//-----------------------------------------------------------------------------

Compiler6502::Compiler6502(InstructionSet cpu_instruction_set,
                           std::ostream *verbose_stream)
    : verbose_stream(verbose_stream) {
    for (auto &[opcode, info] : GetInstructionSet(cpu_instruction_set)) {
        instruction_set[info.mnemonic].variants[info.addres_mode] = info;
    }
}

Compiler6502::~Compiler6502() = default;

std::unique_ptr<Program> Compiler6502::GetProgram() {
    if (!program) {
        throw std::runtime_error("No compiled program available");
    }

    context->UpdateRelocations();

    context.reset();
    return std::move(program);
}

void Compiler6502::Compile(Tokenizer &tokenizer) {
    Start();

    while (tokenizer.HasInput()) {
        auto line = tokenizer.NextLine();
        ProcessLine(line);
    }
}

void Compiler6502::Compile(std::istream &stream, const std::string &name) {
    auto tokenizer = Tokenizer(stream, name);
    Compile(tokenizer);
}

void Compiler6502::CompileString(std::string text, const std::string &name) {
    std::istringstream ss(text);
    ss.exceptions(std::ifstream::badbit);
    Compile(ss, name);
}

void Compiler6502::CompileFile(const std::string &file) {
    std::ifstream ss(file, std::ios::in);
    ss.exceptions(std::ifstream::badbit);
    Compile(ss, file);
}

void Compiler6502::ProcessLine(LineTokenizer &line) {
    while (line.HasInput()) {
        auto first_token = line.NextToken();
        if (!first_token) {
            continue;
        }

        {
            auto first_token_view = first_token.View();
            if (first_token_view.ends_with(":")) {
                context->BeginSymbol(first_token);
                continue;
            }
            if (first_token_view.starts_with(".")) {
                context->HandleCommand(first_token, line);
                continue;
            }
        }
        {
            auto op_handler = instruction_set.find(first_token.Upper());
            if (op_handler != instruction_set.end()) {
                context->EmitInstruction(line, op_handler->second);
                continue;
            }
        }

        if (!line.HasInput()) {
            ThrowCompilationError(CompilationError::InvalidToken, first_token);
        }

        if (TryDefinition(first_token, line)) {
            continue;
        }

        ThrowCompilationError(CompilationError::InvalidToken, first_token);
    }

    if (line.HasInput()) {
        auto t = line.NextToken();
        ThrowCompilationError(CompilationError::UnexpectedInput, t);
    }
}

bool Compiler6502::TryDefinition(const Token &first_token, LineTokenizer &line) {
    auto second_token = line.NextToken();
    if (second_token == "=" || second_token.Lower() == "equ") {
        if (!line.HasInput()) {
            ThrowCompilationError(CompilationError::UnexpectedEndOfInput, second_token);
        }
        auto alias_value = line.NextToken();

        if (line.HasInput()) {
            ThrowCompilationError(CompilationError::UnexpectedInput, line.NextToken());
        }

        context->AddDefinition(first_token, alias_value);
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------

void Compiler6502::Start() {
    if (!program) {
        program = std::make_unique<Program>();
        context = std::make_unique<CompilationContext>(*program, verbose_stream);
    }
}

void Compiler6502::AddDefinitions(const SymbolDefVector &symbols) {
    Start();
    for (auto &item : symbols) {
        context->AddDefinition(item);
    }
}

} // namespace emu::emu6502::assembler
