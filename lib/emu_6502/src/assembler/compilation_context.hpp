#pragma once

#include "emu6502/assembler/compiler.hpp"
#include "emu6502/assembler/tokenizer.hpp"
#include "emu6502/instruction_set.hpp"
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

    void ParseByteCommand(LineTokenizer &tokenizer);
    void ParseWordCommand(LineTokenizer &tokenizer);
    void ParseOriginCommand(LineTokenizer &tokenizer);

    void AddLabel(const std::string &name);

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

    static AddressMode SelectInstuctionVariant(const std::set<AddressMode> &modes,
                                               const InstructionParsingInfo &instruction, std::nullptr_t);
    static AddressMode SelectInstuctionVariant(std::set<AddressMode> modes, const InstructionParsingInfo &instruction,
                                               std::string label);
    static AddressMode SelectInstuctionVariant(const std::set<AddressMode> &modes,
                                               const InstructionParsingInfo &instruction, std::vector<uint8_t> data);

    void ProcessInstructionArgument(const OpcodeInfo &opcode, std::string label);
    void ProcessInstructionArgument(const OpcodeInfo &opcode, std::nullptr_t);
    void ProcessInstructionArgument(const OpcodeInfo &opcode, std::vector<uint8_t> data);
};

} // namespace emu::emu6502::assembler
