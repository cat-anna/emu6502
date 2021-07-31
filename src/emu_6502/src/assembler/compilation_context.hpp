#pragma once

#include "emu_6502/assembler/compilation_error.hpp"
#include "emu_6502/assembler/compiler.hpp"
#include "emu_6502/assembler/tokenizer.hpp"
#include "emu_6502/instruction_set.hpp"
#include "emu_core/program.hpp"
#include "emu_core/symbol_factory.hpp"
#include <fmt/format.h>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

namespace emu::emu6502::assembler {

struct CompilationContext {
    CompilationContext(Program &program, bool verbose = false)
        : program(program), verbose_logs(verbose) {}

    using CommandParserFunc = void (CompilationContext::*)(LineTokenizer &);
    struct CommandParsingInfo {
        CommandParserFunc handler;
    };

    static const std::unordered_map<std::string, CommandParsingInfo> kCommandParseInfo;
    static const std::unordered_map<std::string, Address_t> kIsrMap;

    void HandleCommand(const Token &command_token, LineTokenizer &line_tokenizer);

    void BeginSymbol(const Token &name_token);

    void AddDefinition(const Token &name_token, const Token &value_token);
    void AddDefinition(const SymbolDefinition &symbol);

    void EmitInstruction(LineTokenizer &tokenizer,
                         const InstructionParsingInfo &instruction);

    void UpdateRelocations();

private:
    Program &program;
    const bool verbose_logs = false;
    Address_t current_position = 0;

    template <typename... ARGS>
    void Log(ARGS &&...args) {
        if (verbose_logs) {
            std::cout << "CompilationContext: "
                      << fmt::format(std::forward<ARGS>(args)...) << "\n";
        }
    }

    void ParseOriginCommand(LineTokenizer &tokenizer);
    void ParseTextCommand(LineTokenizer &tokenizer);
    void ParseAlignCommand(LineTokenizer &tokenizer);
    void ParseIsrCommand(LineTokenizer &tokenizer);
    void ParseSymbolCommand(LineTokenizer &tokenizer);

    template <Address_t L>
    void ParseDataCommand(LineTokenizer &tokenizer) {
        ParseDataCommand(tokenizer, L);
    }
    void ParseDataCommand(LineTokenizer &tokenizer, Address_t element_size);

    std::vector<uint8_t> ParseTokenToBytes(const Token &value_token,
                                           size_t expected_byte_size) const;

    void PutSymbolReference(RelocationMode mode, const std::string &symbol,
                            Address_t position);

    void EmitBytes(const ByteVector &data);
};

} // namespace emu::emu6502::assembler
