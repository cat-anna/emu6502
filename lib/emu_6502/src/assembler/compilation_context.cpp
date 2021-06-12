#include "compilation_context.hpp"
#include "emu_core/base16.hpp"
#include "emu_core/byte_utils.hpp"
#include "instruction_argument.hpp"

namespace emu::emu6502::assembler {

const std::unordered_map<std::string, CommandParsingInfo> CompilationContext::kCommandParseInfo = {
    {"byte", {&CompilationContext::ParseDataCommand<1>}},         //
    {"word", {&CompilationContext::ParseDataCommand<2>}},         //
    {"org", {&CompilationContext::ParseOriginCommand}},           //
    {"text", {&CompilationContext::ParseTextCommand}},            //
    {"page_align", {&CompilationContext::ParsePageAlignCommand}}, //
    {"isr", {&CompilationContext::ParseIsrCommand}},              //
};

const std::unordered_map<std::string, Address_t> CompilationContext::kIsrMap = {
    {"reset", kResetVector},
};

void CompilationContext::ParseDataCommand(LineTokenizer &tokenizer, Address_t element_size) {
    for (auto tok : tokenizer.TokenList(",")) {
        switch (GetTokenType(tok, program)) {
        case TokenType::kAlias:
        case TokenType::kValue:
            EmitBytes(ParseTokenToBytes(tok, element_size));
            continue;

        case TokenType::kUnknown:
        case TokenType::kLabel:
            if (element_size == 2) {
                PutLabelReference(RelocationMode::Absolute, tok.String(), current_position);
                EmitBytes(ByteVector(element_size, 0));
                continue;
            } else {
                throw std::runtime_error("ParseDataCommand: Cannot put 1 byte reference to label");
            }
        }

        throw std::runtime_error("ParseDataCommand: Invalid token");
    }
}

void CompilationContext::ParseTextCommand(LineTokenizer &tokenizer) {
    auto tok = tokenizer.NextToken();
    if (tok.value.starts_with('"')) {
        EmitBytes(ParseTextValue(tok, true));
        return;
    }
    throw std::runtime_error("Invalid .text token");
}

void CompilationContext::ParsePageAlignCommand(LineTokenizer &tokenizer) {
    auto new_address = current_position;
    if ((new_address & 0xFF) != 0) {
        new_address &= 0xFF00;
        new_address += kMemoryPageSize;
    }
    Log("Setting position {:04x} -> {:04x}", current_position, new_address);
    current_position = new_address;
}

void CompilationContext::ParseOriginCommand(LineTokenizer &tokenizer) {
    auto tok = tokenizer.NextToken();
    if (tokenizer.HasInput()) {
        throw std::runtime_error("ParseOriginCommand: Unexpected input after .org command");
    }
    switch (GetTokenType(tok, program)) {
    case TokenType::kAlias:
    case TokenType::kValue: {
        auto new_pos = ParseWord(tok.value);
        Log("Setting position {:04x} -> {:04x}", current_position, new_pos);
        current_position = new_pos;
        return;
    }
    case TokenType::kUnknown:
    case TokenType::kLabel:
        throw std::runtime_error("ParseOriginCommand: Cannot change origin to label");
    }

    throw std::runtime_error("ParseOriginCommand: Unknown token type");
}

void CompilationContext::ParseIsrCommand(LineTokenizer &tokenizer) {
    auto isr = tokenizer.NextToken();

    Address_t addr;
    try {
        addr = kIsrMap.at(isr.String());
    } catch (const std::out_of_range &) {
        throw std::runtime_error(fmt::format("Unknown isr '{}'", isr.value));
    }

    auto tok = tokenizer.NextToken();
    Log("Setting ISR {} to '{}'", isr.value, tok.value);

    switch (GetTokenType(tok, program)) {
    case TokenType::kAlias:
        break;
    case TokenType::kValue:
        break;

    case TokenType::kUnknown:
    case TokenType::kLabel:
        PutLabelReference(RelocationMode::Absolute, tok.String(), addr);
        return;
    }
    throw std::runtime_error(fmt::format("Invalid isr command argument '{}'", tok.value));
}

std::vector<uint8_t> CompilationContext::ParseTokenToBytes(const Token &value_token, size_t expected_byte_size) {
    try {
        return ParseImmediateValue(value_token.View(), program.aliases, expected_byte_size);
    } catch (const std::exception &) {
        throw; //TODO: append location and throw proper exception
    }
}

void CompilationContext::AddLabel(const Token &name_token) {
    auto view = name_token.View();
    if (view.ends_with(":")) {
        view.remove_suffix(1);
    }
    auto label_name = std::string(view);

    if (auto label = program.FindLabel(label_name); label == nullptr) {
        Log("Adding label '{}' at {:04x}", label_name, current_position);
        auto l = LabelInfo{
            .name = label_name,
            .offset = current_position,
            .imported = false,
        };
        program.AddLabel(std::make_shared<LabelInfo>(l));
    } else {
        Log("Found label '{}' at {:04x}", label_name, current_position);
        label->imported = false;
        if (label->offset.has_value()) {
            Exception("Label {} is already defined", label_name);
        }

        label->offset = current_position;
        RelocateLabel(*label);
    }
}

void CompilationContext::RelocateLabel(const LabelInfo &label_info) {
    for (auto weak_rel : label_info.label_references) {
        auto rel = weak_rel.lock();
        Log("Relocating reference to label '{}' at {}", label_info.name, to_string(*rel));
        if (rel->mode == RelocationMode::Absolute) {
            auto bytes = ToBytes(current_position);
            program.sparse_binary_code.PutBytes(rel->position, bytes, true);
        } else {
            auto bytes = ToBytes(RelativeJumpOffset(rel->position + 1, current_position));
            program.sparse_binary_code.PutBytes(rel->position, bytes, true);
        }
    }
}

void CompilationContext::EmitInstruction(LineTokenizer &tokenizer, const InstructionParsingInfo &instruction) {
    auto first_token = tokenizer.NextToken();
    auto token = first_token.String();

    if (auto next = tokenizer.NextToken(); next) {
        if (next != ",") {
            throw std::runtime_error("Invalid token");
        }
        auto value = tokenizer.NextToken();
        if (!value) {
            throw std::runtime_error("expected token after ,");
        }
        token += ",";
        token += value.value;
    }

    auto argument = ParseInstructionArgument(token, program.aliases);

    std::vector<AddressMode> instruction_address_modes;
    for (auto &[f, s] : instruction.variants) {
        instruction_address_modes.emplace_back(f);
    }
    std::sort(instruction_address_modes.begin(), instruction_address_modes.end());

    std::set<AddressMode> address_modes;

    std::set_intersection(argument.possible_address_modes.begin(), argument.possible_address_modes.end(), //
                          instruction_address_modes.begin(), instruction_address_modes.end(),             //
                          std::inserter(address_modes, address_modes.begin()));

    AddressMode selected_mode = std::visit(
        [&](auto &arg_value) -> AddressMode { //
            return SelectInstuctionVariant(address_modes, instruction, arg_value);
        },
        argument.argument_value);

    try {
        auto &opcode = instruction.variants.at(selected_mode);
        EmitBytes({opcode.opcode});
        std::visit(
            [&](auto &arg_value) { //
                ProcessInstructionArgument(opcode, arg_value);
            },
            argument.argument_value);
    } catch (const std::exception &e) {
        throw std::runtime_error("opcode does not support mode [TODO] " + std::string(e.what()));
    }
}

AddressMode CompilationContext::SelectInstuctionVariant(const std::set<AddressMode> &modes,
                                                        const InstructionParsingInfo &instruction,
                                                        std::nullptr_t) const {
    if (modes.size() != 1 || *modes.begin() != AddressMode::Implied) {
        throw std::runtime_error("Failed to select implied variant");
    }
    return AddressMode::Implied;
}

AddressMode CompilationContext::SelectInstuctionVariant(std::set<AddressMode> modes,
                                                        const InstructionParsingInfo &instruction,
                                                        std::string label) const {
    if (const auto alias = program.FindAlias(label); alias) {
        modes = FilterPossibleModes(modes, alias->value.size(), true);
    } else {
        modes.erase(AddressMode::ZPX);
        modes.erase(AddressMode::ZPY);
        modes.erase(AddressMode::ZP);
        modes.erase(AddressMode::INDX);
        modes.erase(AddressMode::INDY);
        modes.erase(AddressMode::IM);
        // modes.erase(AddressMode::REL); //TODO?
    }
    if (modes.size() != 1) {
        throw std::runtime_error("Failed to select label variant");
    }
    return *modes.begin();
}

AddressMode CompilationContext::SelectInstuctionVariant(std::set<AddressMode> modes,
                                                        const InstructionParsingInfo &instruction,
                                                        std::vector<uint8_t> data) const {

    auto strict_modes = FilterPossibleModes(modes, data.size(), false);
    if (strict_modes.size() != 1) {
        throw std::runtime_error("Failed to select data variant");
    }
    return *strict_modes.begin();
}

void CompilationContext::ProcessInstructionArgument(const OpcodeInfo &opcode, std::string label) {
    if (const auto alias = program.FindAlias(label); alias) {
        EmitBytes(alias->value);
        return;
    }

    auto mode = opcode.addres_mode == AddressMode::REL ? RelocationMode::Relative : RelocationMode::Absolute;
    PutLabelReference(mode, label, current_position);
    if (mode == RelocationMode::Relative) {
        ++current_position;
    } else {
        current_position += 2;
    }
}

void CompilationContext::ProcessInstructionArgument(const OpcodeInfo &opcode, std::nullptr_t) {
}

void CompilationContext::ProcessInstructionArgument(const OpcodeInfo &opcode, std::vector<uint8_t> data) {
    if (ArgumentByteSize(opcode.addres_mode) < data.size()) {
        throw std::runtime_error("ArgumentByteSize(opcode.addres_mode) - data.size()");
    }
    for (int i = 0, j = static_cast<int>(ArgumentByteSize(opcode.addres_mode) - data.size()); i < j; ++i) {
        program.sparse_binary_code.PutBytes(current_position, {0});
        ++current_position;
    }
    EmitBytes(data);
}

void CompilationContext::PutLabelReference(RelocationMode mode, const std::string &label, Address_t position) {
    auto relocation = std::make_shared<RelocationInfo>();
    relocation->position = position;

    auto existing_it = program.labels.find(label);
    if (existing_it == program.labels.end()) {
        Log("Adding reference at {:04x} to unknown label '{}'", position, label);
        auto l = LabelInfo{
            .name = label,
            .imported = true,
            .label_references = {relocation},
        };
        program.AddLabel(std::make_shared<LabelInfo>(l));
        existing_it = program.labels.find(label);
    } else {
        Log("Adding reference at {:04x} to label '{}'", position, label);
        existing_it->second->label_references.emplace_back(relocation);
    }

    relocation->target_label = existing_it->second;
    auto label_addr = existing_it->second->offset.value_or(position);
    relocation->mode = mode;
    std::vector<uint8_t> bytes;

    if (mode == RelocationMode::Relative) {
        bytes = ToBytes(RelativeJumpOffset(position + 1, label_addr));
    } else {
        bytes = ToBytes(label_addr);
    }

    program.sparse_binary_code.PutBytes(position, bytes);
    program.relocations.insert(relocation);
}

void CompilationContext::AddAlias(const Token &name_token, const Token &value_token) {
    auto data = ParsePackedIntegral(value_token.View());
    Log("Adding alias '{}' = '{}'", name_token.String(), ToHex(data, ""));
    program.AddAlias(std::make_shared<ValueAlias>(ValueAlias{name_token.String(), data}));
}

void CompilationContext::EmitBytes(const ByteVector &data) {
    program.sparse_binary_code.PutBytes(current_position, data);
    current_position += static_cast<Address_t>(data.size());
}

} // namespace emu::emu6502::assembler
