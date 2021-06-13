#include "emu_6502/assembler/compiler.hpp"
#include "compilation_context.hpp"
#include "emu_6502/assembler/compilation_error.hpp"
#include <fstream>
#include <sstream>

namespace emu::emu6502::assembler {

std::unique_ptr<Program> Compiler6502::CompileString(std::string text, InstructionSet cpu_instruction_set) {
    std::istringstream ss(text);
    // ss.exceptions(std::ifstream::failbit);
    Compiler6502 c{cpu_instruction_set};
    auto tokenizer = Tokenizer(ss, "string");
    return c.Compile(tokenizer);
}

std::unique_ptr<Program> Compiler6502::CompileFile(const std::string &file, InstructionSet cpu_instruction_set) {
    std::ifstream ss(file, std::ios::in);
    ss.exceptions(std::ifstream::failbit);
    Compiler6502 c{cpu_instruction_set};
    auto tokenizer = Tokenizer(ss, file);
    return c.Compile(tokenizer);
}

//-----------------------------------------------------------------------------

Compiler6502::Compiler6502(InstructionSet cpu_instruction_set) {
    for (auto &[opcode, info] : GetInstructionSet(cpu_instruction_set)) {
        instruction_set[info.mnemonic].variants[info.addres_mode] = info;
    }
}

std::unique_ptr<Program> Compiler6502::Compile(Tokenizer &tokenizer) {
    std::unique_ptr<Program> program = std::make_unique<Program>();
    CompilationContext context{*program, true};

    while (tokenizer.HasInput()) {
        auto line = tokenizer.NextLine();
        ProcessLine(context, line);
    }

    return program;
}

void Compiler6502::ProcessLine(CompilationContext &context, LineTokenizer &line) {
    while (line.HasInput()) {
        auto first_token = line.NextToken();
        if (!first_token) {
            continue;
        }

        {
            auto first_token_view = first_token.View();
            if (first_token_view.ends_with(":")) {
                context.AddLabel(first_token);
                continue;
            }

            if (first_token_view.starts_with(".")) {
                context.HandleCommand(first_token, line);
                continue;
            }
        }

        auto op_handler = instruction_set.find(first_token.Upper());
        if (op_handler != instruction_set.end()) {
            context.EmitInstruction(line, op_handler->second);
            continue;
        }

        if (!line.HasInput()) {
            ThrowCompilationError(CompilationError::InvalidToken, first_token);
        }

        auto second_token = line.NextToken();
        if (second_token == "=" || second_token.Lower() == "equ") {
            if (!line.HasInput()) {
                ThrowCompilationError(CompilationError::UnexpectedEndOfInput, second_token);
            }
            auto alias_value = line.NextToken();

            if (line.HasInput()) {
                ThrowCompilationError(CompilationError::UnexpectedInput, line.NextToken());
            }

            context.AddAlias(first_token, alias_value);
            continue;
        }

        ThrowCompilationError(CompilationError::InvalidToken, first_token);
    }

    if (line.HasInput()) {
        auto t = line.NextToken();
        ThrowCompilationError(CompilationError::UnexpectedInput, t);
    }
}

} // namespace emu::emu6502::assembler
