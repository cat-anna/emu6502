#pragma once

#include "emu_6502/assembler/compiler.hpp"
#include "emu_6502/assembler/tokenizer.hpp"
#include "emu_6502/instruction_set.hpp"
#include "emu_core/program.hpp"
#include <fmt/format.h>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

namespace emu::emu6502::assembler {

struct CompilationContext;

using CommandParserFunc = void (CompilationContext::*)(LineTokenizer &);

struct CommandParsingInfo {
    CommandParserFunc handler;
};

struct CompilationContext {
    Program &program;
    const bool verbose_logs = false;

    Address_t current_position = 0;

    CompilationContext(Program &program, bool verbose = false) : program(program), verbose_logs(verbose) {}

    static const std::unordered_map<std::string, CommandParsingInfo> kCommandParseInfo;

    void AddLabel(const std::string &name);
    void AddAlias(const std::string &name, const Token &value_token);

    void ParseInstruction(LineTokenizer &tokenizer, const InstructionParsingInfo &instruction);

private:
    void RelocateLabel(const LabelInfo &label_info);

    template <typename... ARGS>
    void Log(ARGS &&... args) {
        if (verbose_logs) {
            std::cout << "CompilationContext: " << fmt::format(std::forward<ARGS>(args)...) << "\n";
        }
    }

    template <typename... ARGS>
    [[noreturn]] void Exception(ARGS &&... args) {
        auto err = fmt::format(std::forward<ARGS>(args)...);
        if (verbose_logs) {
            std::cout << "CompilationContext: Error:" << err << "\n";
        }
        throw std::runtime_error(err);
    }

    AddressMode SelectInstuctionVariant(const std::set<AddressMode> &modes, const InstructionParsingInfo &instruction,
                                        std::nullptr_t) const;
    AddressMode SelectInstuctionVariant(std::set<AddressMode> modes, const InstructionParsingInfo &instruction,
                                        std::string label) const;
    AddressMode SelectInstuctionVariant(const std::set<AddressMode> &modes, const InstructionParsingInfo &instruction,
                                        std::vector<uint8_t> data) const;

    void ProcessInstructionArgument(const OpcodeInfo &opcode, std::nullptr_t);
    void ProcessInstructionArgument(const OpcodeInfo &opcode, std::vector<uint8_t> data);
    void ProcessInstructionArgument(const OpcodeInfo &opcode, std::string label);

    void ParseOriginCommand(LineTokenizer &tokenizer);
    void ParseTextCommand(LineTokenizer &tokenizer);
    void ParsePageAlignCommand(LineTokenizer &tokenizer);
    void ParseIsrCommand(LineTokenizer &tokenizer);

    template <Address_t L>
    void ParseDataCommand(LineTokenizer &tokenizer) {
        ParseDataCommand(tokenizer, L);
    }
    void ParseDataCommand(LineTokenizer &tokenizer, Address_t element_size);

    std::vector<uint8_t> ParseTokenToBytes(const Token &value_token, size_t expected_byte_size);

    void PutLabelReference(RelocationMode mode, const std::string &label, Address_t position);
};

} // namespace emu::emu6502::assembler
