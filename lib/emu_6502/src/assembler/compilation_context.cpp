#include "compilation_context.hpp"
#include "emu_core/byte_utils.hpp"
#include "instruction_argument.hpp"

namespace emu::emu6502::assembler {

const std::unordered_map<std::string, CommandParsingInfo> CompilationContext::kCommandParseInfo = {
    {"byte", {&CompilationContext::ParseByteCommand}},            //
    {"word", {&CompilationContext::ParseWordCommand}},            //
    {"org", {&CompilationContext::ParseOriginCommand}},           //
    {"text", {&CompilationContext::ParseTextCommand}},            //
    {"page_align", {&CompilationContext::ParsePageAlignCommand}}, //
    {"isr_reset", {&CompilationContext::ParseResetCommand}},      //
};

void CompilationContext::ParseByteCommand(LineTokenizer &tokenizer) {
    while (tokenizer.HasInput()) {
        auto tok = tokenizer.NextToken();
        program.sparse_binary_code.PutBytes(current_position, ToBytes(ParseByte(tok.value)));
        ++current_position;
    }
}

void CompilationContext::ParseWordCommand(LineTokenizer &tokenizer) {
    while (tokenizer.HasInput()) {
        auto tok = tokenizer.NextToken();
        program.sparse_binary_code.PutBytes(current_position, ToBytes(ParseWord(tok.value)));
        current_position += 2;
    }
}

void CompilationContext::ParseTextCommand(LineTokenizer &tokenizer) {
    auto tok = tokenizer.NextToken();
    if (tok.value.starts_with('"')) {
        auto token = tok.value;
        token.remove_prefix(1);
        token.remove_suffix(1);
        program.sparse_binary_code.PutBytes(current_position, ToBytes(token));
        current_position += static_cast<Address_t>(token.size());
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
    auto new_pos = ParseWord(tok.value);
    Log("Setting position {:04x} -> {:04x}", current_position, new_pos);
    current_position = new_pos;
}

void CompilationContext::ParseResetCommand(LineTokenizer &tokenizer) {
    auto tok = tokenizer.NextToken();
    PutLabelReference(false, tok.String(), kResetVector);
}

void CompilationContext::AddLabel(const std::string &name) {
    if (auto it = program.labels.find(name); it == program.labels.end()) {
        Log("Adding label '{}' at {:04x}", name, current_position);
        auto l = LabelInfo{
            .name = name,
            .offset = current_position,
            .imported = false,
        };
        program.labels[l.name] = std::make_shared<LabelInfo>(l);
    } else {
        Log("Found label '{}' at {:04x}", name, current_position);
        it->second->imported = false;
        if (it->second->offset.has_value()) {
            Exception("Label {} is already defined", name);
        }

        it->second->offset = current_position;
        RelocateLabel(*it->second);
    }
}

void CompilationContext::RelocateLabel(const LabelInfo &label_info) {
    for (auto weak_rel : label_info.label_references) {
        auto rel = weak_rel.lock();
        Log("Relocating reference to {} at {}", label_info.name, to_string(*rel));
        if (rel->mode == RelocationMode::Absolute) {
            auto bytes = ToBytes(current_position);
            program.sparse_binary_code.PutBytes(rel->position, bytes, true);
        } else {
            auto bytes = ToBytes(RelativeJumpOffset(rel->position + 1, current_position));
            program.sparse_binary_code.PutBytes(rel->position, bytes, true);
        }
    }
}

void CompilationContext::ParseInstruction(LineTokenizer &tokenizer, const InstructionParsingInfo &instruction) {
    auto first_token = tokenizer.NextToken();
    auto token = first_token.String();

    if (auto next = tokenizer.NextToken(); next) {
        token += ",";
        token += next.String();
    }

    auto argument = ParseInstructionArgument(token);

    std::vector<AddressMode> instruction_address_modes;
    for (auto &[f, s] : instruction.variants) {
        instruction_address_modes.emplace_back(f);
    }
    std::sort(instruction_address_modes.begin(), instruction_address_modes.end());
    std::sort(argument.possible_address_modes.begin(), argument.possible_address_modes.end());

    std::set<AddressMode> address_modes;

    std::set_intersection(argument.possible_address_modes.begin(), argument.possible_address_modes.end(), //
                          instruction_address_modes.begin(), instruction_address_modes.end(),             //
                          std::inserter(address_modes, address_modes.begin()));

    AddressMode selected_mode = std::visit(
        [&](auto &arg_value) -> AddressMode { return SelectInstuctionVariant(address_modes, instruction, arg_value); },
        argument.argument_value);

    try {
        auto &opcode = instruction.variants.at(selected_mode);
        program.sparse_binary_code.PutByte(current_position++, opcode.opcode);
        std::visit([&](auto &arg_value) { ProcessInstructionArgument(opcode, arg_value); }, argument.argument_value);
    } catch (const std::exception &e) {
        throw std::runtime_error("opcode does not support mode [TODO] " + std::string(e.what()));
    }
}

AddressMode CompilationContext::SelectInstuctionVariant(const std::set<AddressMode> &modes,
                                                        const InstructionParsingInfo &instruction, std::nullptr_t) {
    if (modes.size() != 1 || *modes.begin() != AddressMode::Implied) {
        throw std::runtime_error("Failed to select implied variant");
    }
    return AddressMode::Implied;
}

AddressMode CompilationContext::SelectInstuctionVariant(std::set<AddressMode> modes,
                                                        const InstructionParsingInfo &instruction, std::string label) {
    modes.erase(AddressMode::ZPX); // TODO: not (yet) supported for labels/aliases
    modes.erase(AddressMode::ZPY);
    modes.erase(AddressMode::ZP);
    if (modes.size() != 1) {
        throw std::runtime_error("Failed to select label variant");
    }
    return *modes.begin();
}

AddressMode CompilationContext::SelectInstuctionVariant(const std::set<AddressMode> &modes,
                                                        const InstructionParsingInfo &instruction,
                                                        std::vector<uint8_t> data) {
    if (modes.size() != 1) {
        throw std::runtime_error("Failed to select data variant");
    }
    return *modes.begin();
}

void CompilationContext::ProcessInstructionArgument(const OpcodeInfo &opcode, std::string label) {
    bool relative = opcode.addres_mode == AddressMode::REL;
    PutLabelReference(relative, label, current_position);
    if (relative) {
        ++current_position;
    } else {
        current_position += 2;
    }
}

void CompilationContext::ProcessInstructionArgument(const OpcodeInfo &opcode, std::nullptr_t) {
}

void CompilationContext::ProcessInstructionArgument(const OpcodeInfo &opcode, std::vector<uint8_t> data) {
    program.sparse_binary_code.PutBytes(current_position, data);
    current_position += static_cast<Address_t>(data.size());
}

void CompilationContext::PutLabelReference(bool relative, const std::string &label, Address_t position) {
    auto relocation = std::make_shared<RelocationInfo>();
    relocation->position = position;

    auto existing_it = program.labels.find(label);
    if (existing_it == program.labels.end()) {
        std::cout << "ADDING LABEL FORWARD REF " << label << " at " << position << "\n";
        auto l = LabelInfo{
            .name = label,
            .imported = true,
            .label_references = {relocation},
        };
        program.labels[l.name] = std::make_shared<LabelInfo>(l);
        existing_it = program.labels.find(label);
    } else {
        std::cout << "ADDING LABEL REF " << label << " at " << position << "\n";
        existing_it->second->label_references.emplace_back(relocation);
    }

    relocation->target_label = existing_it->second;
    auto label_addr = existing_it->second->offset.value_or(position);
    if (relative) {
        auto bytes = ToBytes(RelativeJumpOffset(position + 1, label_addr));
        relocation->mode = RelocationMode::Relative;
        program.sparse_binary_code.PutBytes(position, bytes);
    } else {
        relocation->mode = RelocationMode::Absolute;
        auto bytes = ToBytes(label_addr);
        program.sparse_binary_code.PutBytes(position, bytes);
    }

    program.relocations.insert(relocation);
}

} // namespace emu::emu6502::assembler
