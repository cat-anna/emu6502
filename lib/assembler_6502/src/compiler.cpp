#include "assembler_6502/compiler.hpp"
#include "compilation_context.hpp"
#include <fstream>
#include <sstream>

namespace emu::assembler6502 {

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

Compiler6502::Compiler6502(cpu6502::InstructionSet cpu_instruction_set) {
    for (auto &[opcode, info] : cpu6502::GetInstructionSet(cpu_instruction_set)) {
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
        auto token = line.NextToken();
        if (!token) {
            continue;
        }

        if (token.value.ends_with(":")) {
            token.value.remove_suffix(1);
            context.AddLabel(token.String());
            continue;
        }

        if (token.value.starts_with(".")) {
            token.value.remove_prefix(1);
            auto parse_info_it = CompilationContext::kCommandParseInfo.find(token.String());
            if (parse_info_it == CompilationContext::kCommandParseInfo.end()) {
                throw std::runtime_error(fmt::format("Failed to parse .{} command", token));
            } else {
                auto handler = parse_info_it->second.handler;
                (context.*handler)(line);
            }
            continue;
        }

        auto op_handler = instruction_set.find(token.Upper());
        if (op_handler != instruction_set.end()) {
            context.ParseInstruction(line, op_handler->second);
            continue;
        }

        throw std::runtime_error(fmt::format("Unknown {}", to_string(token)));
    }
}

} // namespace emu::assembler6502
