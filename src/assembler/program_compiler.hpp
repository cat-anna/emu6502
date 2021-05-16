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

namespace emu::assembler {

namespace opcode = emu::cpu6502::opcode;

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
                throw std::runtime_error("Unknown token '" + upper + "'");
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
            std::cout << "FOUND LABEL " << name << " at " << current_position << "\n";
            it->second->imported = false;
            if (it->second->offset.has_value()) {
                throw std::runtime_error("Label " + std::string(name) + " is already defined");
            }
            it->second->offset = current_position;
            for (auto weak_rel : it->second->label_references) {
                auto rel = weak_rel.lock();
                if (rel->mode == RelocationMode::Absolute) {
                    auto bytes = ToBytes(current_position);
                    program.sparse_binary_code.PutBytes(rel->position, bytes, true);
                } else {
                    auto bytes = ToBytes(RelativeJumpOffset(rel->position + 1, current_position));
                    program.sparse_binary_code.PutBytes(rel->position, bytes, true);
                }
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
        current_position = new_pos;
    }

    using OpcodeInfo = emu::cpu6502::opcode::OpcodeInfo;
    using AddressMode = emu::cpu6502::opcode::AddressMode;

    void ParseInstruction(Program &program, const NextTokenFunc &get_next_token,
                          const InstructionParsingInfo &instruction) {
        auto token = std::string(get_next_token().value_or(""));

        if (auto next_token = get_next_token(); next_token.has_value() && !next_token->empty()) {
            token += ",";
            token += next_token.value();
        }

        auto argument = ParseInstructionArgument(std::string_view(token));

        std::vector<AddressMode> instruction_address_modes;
        for (auto &[f, s] : instruction.variants) {
            instruction_address_modes.emplace_back(f);
        }
        std::sort(instruction_address_modes.begin(), instruction_address_modes.end());
        std::sort(argument.possible_address_modes.begin(), argument.possible_address_modes.end());

        std::set<AddressMode> address_modes;

        std::set_intersection(argument.possible_address_modes.begin(), argument.possible_address_modes.end(), //
                              instruction_address_modes.begin(), instruction_address_modes.end(),
                              std::inserter(address_modes, address_modes.begin()));

        AddressMode selected_mode = std::visit(
            [&](auto &arg_value) -> AddressMode {
                return SelectInstuctionVariant(program, address_modes, instruction, arg_value);
            },
            argument.argument_value);

        try {
            auto &opcode = instruction.variants.at(selected_mode);
            program.sparse_binary_code.PutByte(current_position++, opcode.opcode);
            std::visit([&](auto &arg_value) { ProcessInstructionArgument(program, opcode, arg_value); },
                       argument.argument_value);
        } catch (const std::exception &e) {
            throw std::runtime_error("opcode does not support mode [TODO] " + std::string(e.what()));
        }
    }

    AddressMode SelectInstuctionVariant(Program &program, const std::set<AddressMode> &modes,
                                        const InstructionParsingInfo &instruction, std::nullptr_t) {
        if (modes.size() != 1 || *modes.begin() != AddressMode::Implied) {
            throw std::runtime_error("Failed to select implied variant");
        }
        return AddressMode::Implied;
    }

    AddressMode SelectInstuctionVariant(Program &program, std::set<AddressMode> modes,
                                        const InstructionParsingInfo &instruction, std::string label) {
        modes.erase(AddressMode::ZPX); // TODO: not yet supported for labels/aliases
        modes.erase(AddressMode::ZPY);
        modes.erase(AddressMode::ZP);
        if (modes.size() != 1) {
            throw std::runtime_error("Failed to select label variant");
        }
        return *modes.begin();
    }

    AddressMode SelectInstuctionVariant(Program &program, const std::set<AddressMode> &modes,
                                        const InstructionParsingInfo &instruction, std::vector<uint8_t> data) {
        if (modes.size() != 1) {
            throw std::runtime_error("Failed to select data variant");
        }
        return *modes.begin();
    }

    void ProcessInstructionArgument(Program &program, const OpcodeInfo &opcode, std::string label) {

        auto relocation = std::make_shared<RelocationInfo>();
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
        auto label_addr = existing_it->second->offset.value_or(current_position);
        if (opcode.addres_mode == AddressMode::REL) {
            auto bytes = ToBytes(RelativeJumpOffset(current_position + 1, label_addr));
            relocation->mode = RelocationMode::Relative;
            ProcessInstructionArgument(program, opcode, bytes);
        } else {
            relocation->mode = RelocationMode::Absolute;
            ProcessInstructionArgument(program, opcode, ToBytes(label_addr));
        }

        program.relocations.insert(relocation);
    }

    void ProcessInstructionArgument(Program &program, const OpcodeInfo &opcode, std::nullptr_t) {}
    void ProcessInstructionArgument(Program &program, const OpcodeInfo &opcode, std::vector<uint8_t> data) {
        program.sparse_binary_code.PutBytes(current_position, data);
        current_position += data.size();
    }
};

} // namespace emu::assembler