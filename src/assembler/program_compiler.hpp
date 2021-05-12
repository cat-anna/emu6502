#pragma once

#include "assember.hpp"
#include "text_utils.hpp"
#include <algorithm>
#include <cpu/opcode.hpp>
#include <functional>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace emu6502::assembler {

namespace opcode = emu6502::cpu::opcode;

struct InstructionParsingInfo {
    std::unordered_map<opcode::AddressMode, opcode::OpcodeInfo> variants;
};

using NextTokenFunc = std::function<std::optional<std::string_view>()>;
struct ProgramCompiler;
using CommandParserFunc = void (ProgramCompiler::*)(Program &, const NextTokenFunc &);

struct CommandParsingInfo {
    CommandParserFunc handler;
};

struct ProgramCompiler {
    Address_t current_position = 0;

    std::unordered_map<std::string, CommandParsingInfo> command_parse_info;
    std::unordered_map<std::string_view, InstructionParsingInfo> instruction_set;

    ProgramCompiler() {
        command_parse_info = {
            {"byte", {&ProgramCompiler::ParseByteCommand}},
            {"word", {&ProgramCompiler::ParseWordCommand}},
            {"org", {&ProgramCompiler::ParseOriginCommand}},
        };

        for (auto &[opcode, info] : opcode::Get6502InstructionSet()) {
            instruction_set[info.mnemonic].variants[info.addres_mode] = info;
        }
    }

    void NextLine(Program &program, std::string_view line) {
        auto tokens = Tokenize(line);
        NextTokenFunc get_next_token = [&, next_token = tokens.begin()]() mutable -> std::optional<std::string_view> {
            if (next_token == tokens.end()) {
                return std::nullopt;
            }
            auto token = *next_token;
            ++next_token;
            // std::cout << "NEXT TOKEN: " << token << "\n";
            return token;
        };

        for (auto next_token = get_next_token(); next_token.has_value(); next_token = get_next_token()) {
            auto tok = next_token.value();
            if (tok.ends_with(":")) {
                tok.remove_suffix(1);
                AddLabel(program, tok);
                continue;
            }

            if (tok.starts_with(".")) {
                tok.remove_prefix(1);
                try {
                    auto &parse_info = command_parse_info.at(std::string(tok));
                    (this->*(parse_info.handler))(program, get_next_token);
                } catch (const std::exception &e) {
                    throw std::runtime_error("Failed to parse ." + std::string(tok) + " command");
                }
                continue;
            }

            std::string upper;
            transform(tok.begin(), tok.end(), std::back_inserter(upper), ::toupper);
            auto op_handler = instruction_set.find(upper);
            if (op_handler == instruction_set.end()) {
                throw std::runtime_error("Unknown token " + upper);
            }
            ParseInstruction(program, get_next_token, op_handler->second);
        }
    }

private:
    void AddLabel(Program &program, std::string_view name) {
        if (auto it = program.labels.find(std::string(name)); it == program.labels.end()) {
            std::cout << "ADDING LABEL " << name << " at " << current_position << "\n";
            auto l = LabelInfo{
                .name = std::string(name),
                .offset = current_position,
                .imported = false,
            };
            program.labels[l.name] = std::make_shared<LabelInfo>(l);
        } else {
            it->second->imported = false;
            if (it->second->offset.has_value()) {
                throw std::runtime_error("Label " + std::string(name) + " is already defined");
            }
            it->second->offset = current_position;
            auto pos = ToBytes(current_position);
            for (auto weak_rel : it->second->label_references) {
                auto rel = weak_rel.lock();
                program.sparse_binary_code.PutBytes(rel->position, pos, true);
            }
        }
    }

    void ParseByteCommand(Program &program, const NextTokenFunc &get_next_token) {
        for (auto token = get_next_token(); token.has_value(); token = get_next_token()) {
            program.sparse_binary_code.PutByte(current_position, ParseByte(token.value()));
            ++current_position;
        }
    }

    void ParseWordCommand(Program &program, const NextTokenFunc &get_next_token) {
        for (auto token = get_next_token(); token.has_value(); token = get_next_token()) {
            program.sparse_binary_code.PutBytes(current_position, ToBytes(ParseWord(token.value())));
            ++current_position;
        }
    }
    void ParseOriginCommand(Program &program, const NextTokenFunc &get_next_token) {
        auto new_pos = ParseWord(get_next_token().value());
        std::cout << fmt::format("ORG {:04x} -> {:04x}\n", current_position, new_pos);
        current_position = new_pos;
    }

    using OpcodeInfo = emu6502::cpu::opcode::OpcodeInfo;
    using AddressMode = emu6502::cpu::opcode::AddressMode;

    void ParseInstruction(Program &program, const NextTokenFunc &get_next_token,
                          const InstructionParsingInfo &instruction) {
        auto next_token = get_next_token();
        auto argument = ParseInstructionArgument(next_token.value_or(""));

        std::vector<AddressMode> instruction_address_modes;
        for (auto &[f, s] : instruction.variants) {
            instruction_address_modes.emplace_back(f);
        }
        std::sort(instruction_address_modes.begin(), instruction_address_modes.end());
        std::sort(argument.possible_address_modes.begin(), argument.possible_address_modes.end());

        std::vector<AddressMode> address_modes;

        std::set_intersection(argument.possible_address_modes.begin(), argument.possible_address_modes.end(), //
                              instruction_address_modes.begin(), instruction_address_modes.end(),
                              std::back_inserter(address_modes));

        if (address_modes.size() != 1) {
            throw std::runtime_error("Not impl");
        }

        try {
            auto &opcode = instruction.variants.at(address_modes[0]);
            program.sparse_binary_code.PutByte(current_position++, opcode.opcode);
            std::visit([&](auto &arg_value) { ProcessInstructionArgument(program, opcode, arg_value); },
                       argument.argument_value);
        } catch (const std::exception &e) {
            throw std::runtime_error("opcode does not support mode [TODO]");
        }
    }

    void ProcessInstructionArgument(Program &program, const OpcodeInfo &opcode, std::string label) {

        auto relocation = std::make_shared<RelocationInfo>();
        program.relocations.insert(relocation);

        relocation->position = current_position;

        auto existing_it = program.labels.find(label);
        if (existing_it == program.labels.end()) {
            std::cout << "ADDING LABEL FORWARD REF " << label << " at " << current_position << "\n";
            auto l = LabelInfo{
                .name = label,
                .imported = true,
                .label_references = {relocation},
            };
            program.labels[l.name] = std::make_shared<LabelInfo>(l);
            existing_it = program.labels.find(label);
        } else {
            std::cout << "ADDING LABEL REF " << label << " at " << current_position << "\n";
            existing_it->second->label_references.emplace_back(relocation);
        }

        relocation->target_label = existing_it->second;
        auto label_addr = existing_it->second->offset.value_or(0);
        if (opcode.addres_mode == AddressMode::REL) {
            int16_t rel = current_position - label_addr;
            relocation->mode = RelocationMode::Relative;
            ProcessInstructionArgument(program, opcode, ToBytes(static_cast<uint8_t>(rel)));
        } else {
            relocation->mode = RelocationMode::Absolute;
            ProcessInstructionArgument(program, opcode, ToBytes(existing_it->second->offset.value_or(0)));
        }
    }

    void ProcessInstructionArgument(Program &program, const OpcodeInfo &opcode, std::nullptr_t) {}
    void ProcessInstructionArgument(Program &program, const OpcodeInfo &opcode, std::vector<uint8_t> data) {
        program.sparse_binary_code.PutBytes(current_position, data);
        current_position += data.size();
    }
};

} // namespace emu6502::assembler