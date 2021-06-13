#include "compilation_context.hpp"
#include "emu_core/base16.hpp"
#include "emu_core/byte_utils.hpp"
#include "instruction_argument.hpp"
#include "instrution_variant_compiler.hpp"

namespace emu::emu6502::assembler {

const std::unordered_map<std::string, CompilationContext::CommandParsingInfo> CompilationContext::kCommandParseInfo = {
    {"byte", {&CompilationContext::ParseDataCommand<1>}},         //
    {"word", {&CompilationContext::ParseDataCommand<2>}},         //
    {"org", {&CompilationContext::ParseOriginCommand}},           //
    {"text", {&CompilationContext::ParseTextCommand}},            //
    {"page_align", {&CompilationContext::ParsePageAlignCommand}}, //
    {"isr", {&CompilationContext::ParseIsrCommand}},              //
};

const std::unordered_map<std::string, Address_t> CompilationContext::kIsrMap = {
    {"reset", kResetVector},
    {"irq", kIrqVector},
    {"nmib", kNmibVector},
};

void CompilationContext::HandleCommand(const Token &command_token, LineTokenizer &line_tokenizer) {
    auto command = command_token.View();
    command.remove_prefix(1);
    if (auto it = kCommandParseInfo.find(std::string(command)); it == kCommandParseInfo.end()) {
        ThrowCompilationError(CompilationError::UnknownCommand, command_token);
    } else {
        auto handler = it->second.handler;
        (this->*handler)(line_tokenizer);
    }
}

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
                continue;
            } else {
                ThrowCompilationError(CompilationError::InvalidOperandSize, tok,
                                      "Cannot put 1 byte reference to label");
            }
        }

        ThrowCompilationError(CompilationError::InvalidToken, tok);
    }
}

void CompilationContext::ParseTextCommand(LineTokenizer &tokenizer) {
    auto tok = tokenizer.NextToken();
    if (tok.value.starts_with('"')) {
        EmitBytes(ParseTextValue(tok, true));
        return;
    }

    ThrowCompilationError(CompilationError::InvalidToken, tok);
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
        ThrowCompilationError(CompilationError::LabelIsNotAllowed, tok);
    }

    ThrowCompilationError(CompilationError::InvalidToken, tok);
}

void CompilationContext::ParseIsrCommand(LineTokenizer &tokenizer) {
    auto isr = tokenizer.NextToken();

    Address_t addr;
    try {
        addr = kIsrMap.at(isr.String());
    } catch (const std::out_of_range &) {
        ThrowCompilationError(CompilationError::UnknownIsr, isr);
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

    ThrowCompilationError(CompilationError::InvalidIsrArgument, tok, "Unknown isr command argument '{}'", tok.String());
}

std::vector<uint8_t> CompilationContext::ParseTokenToBytes(const Token &value_token, size_t expected_byte_size) {
    try {
        return ParseImmediateValue(value_token.View(), program.aliases, expected_byte_size);
    } catch (const std::exception &e) {
        ThrowCompilationError(CompilationError::InvalidToken, value_token, e.what());
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
            ThrowCompilationError(CompilationError::LabelRedefinition, name_token);
        }

        label->offset = current_position;
        RelocateLabel(*label);
    }
}

void CompilationContext::ApplyRelocation(const RelocationInfo &relocation, const LabelInfo &label_info) {
    Log("Relocating reference to label '{}' at {}", label_info.name, to_string(relocation));
    if (relocation.mode == RelocationMode::Absolute) {
        auto bytes = ToBytes(label_info.offset.value_or(0));
        program.sparse_binary_code.PutBytes(relocation.position, bytes, true);
    } else {
        auto jump = RelativeJumpOffset(relocation.position + 1, label_info.offset.value_or(relocation.position));
        program.sparse_binary_code.PutBytes(relocation.position, ToBytes(jump), true);
    }
}

void CompilationContext::RelocateLabel(const LabelInfo &label_info) {
    for (auto weak_rel : label_info.label_references) {
        auto rel = weak_rel.lock();
        ApplyRelocation(*rel, label_info);
    }
}

void CompilationContext::EmitInstruction(LineTokenizer &tokenizer, const InstructionParsingInfo &instruction) {
    auto first_token = tokenizer.NextToken();
    auto token = first_token.String();

    if (auto next = tokenizer.NextToken(); next) {
        if (next != ",") {
            ThrowCompilationError(CompilationError::InvalidToken, next);
        }
        auto value = tokenizer.NextToken();
        if (!value) {
            ThrowCompilationError(CompilationError::UnexpectedEndOfInput, value);
        }
        token += ",";
        token += value.value;
    }

    Token meta_token{first_token.line, first_token.location, token};
    auto argument = ParseInstructionArgument(meta_token, program.aliases);

    std::set<AddressMode> instruction_address_modes;
    for (auto &[f, s] : instruction.variants) {
        instruction_address_modes.emplace(f);
    }

    InstructionVariantSelector selector{
        .possible_address_modes = {},
        .token = first_token,
        .aliases = program.aliases,
    };

    std::set_intersection(argument.possible_address_modes.begin(), argument.possible_address_modes.end(), //
                          instruction_address_modes.begin(), instruction_address_modes.end(),             //
                          std::inserter(selector.possible_address_modes, selector.possible_address_modes.begin()));

    AddressMode selected_mode = selector.DispatchSelect(argument.argument_value);

    try {
        auto iadp = InstructionArgumentDataProcessor{
            .opcode = instruction.variants.at(selected_mode),
            .token = first_token,
            .current_position = current_position,
        };
        auto r = iadp.DispatchProcess(argument.argument_value);
        EmitBytes(r.bytes);
        if (r.relocation_mode.has_value()) {
            PutLabelReference(*r.relocation_mode, r.relocation_label, r.relocation_position);
        }
    } catch (const std::exception &e) {
        ThrowCompilationError(CompilationError::InvalidOperandArgument, first_token, "{}", e.what());
    }
}

void CompilationContext::PutLabelReference(RelocationMode mode, const std::string &label, Address_t position) {
    auto relocation = std::make_shared<RelocationInfo>();
    std::shared_ptr<LabelInfo> label_ptr = program.FindLabel(label);
    if (label_ptr == nullptr) {
        Log("Adding reference at {:04x} to unknown label '{}'", position, label);
        auto l = LabelInfo{
            .name = label,
            .imported = true,
            .label_references = {relocation},
        };
        program.AddLabel(label_ptr = std::make_shared<LabelInfo>(l));
    } else {
        Log("Adding reference at {:04x} to label '{}'", position, label);
        label_ptr->label_references.emplace_back(relocation);
    }

    relocation->position = position;
    relocation->mode = mode;
    relocation->target_label = label_ptr;
    program.relocations.insert(relocation);

    ApplyRelocation(*relocation, *label_ptr);
}

void CompilationContext::AddAlias(const Token &name_token, const Token &value_token) {
    auto data = ParsePackedIntegral(value_token.View());
    Log("Adding alias '{}' = '{}'", name_token.String(), ToHex(data, ""));
    if (program.FindAlias(name_token.String()) != nullptr) {
        ThrowCompilationError(CompilationError::AliasRedefinition, name_token);
    }
    program.AddAlias(std::make_shared<ValueAlias>(ValueAlias{name_token.String(), data}));
}

void CompilationContext::EmitBytes(const ByteVector &data) {
    program.sparse_binary_code.PutBytes(current_position, data);
    current_position += static_cast<Address_t>(data.size());
}

} // namespace emu::emu6502::assembler
