#include "compilation_context.hpp"
#include "emu_core/base16.hpp"
#include "emu_core/byte_utils.hpp"
#include "instruction_argument.hpp"
#include "instruction_variant_compiler.hpp"
#include <bit>

namespace emu::emu6502::assembler {

const std::unordered_map<std::string, CompilationContext::CommandParsingInfo>
    CompilationContext::kCommandParseInfo = {
        //cc65
        {"addr", {&CompilationContext::ParseDataCommand<2>}},  //
        {"align", {&CompilationContext::ParseAlignCommand}},   //
        {"asciiz", {&CompilationContext::ParseTextCommand}},   //
        {"byt", {&CompilationContext::ParseDataCommand<1>}},   //
        {"byte", {&CompilationContext::ParseDataCommand<1>}},  //
        {"dbyt", {&CompilationContext::ParseDataCommand<2>}},  //
        {"dword", {&CompilationContext::ParseDataCommand<4>}}, //
        {"org", {&CompilationContext::ParseOriginCommand}},    //
        {"word", {&CompilationContext::ParseDataCommand<2>}},  //

        // {"autoimport", {nullptr}},  //
        // {"blankbytes", {nullptr}},  //
        // {"case", {nullptr}},        //
        // {"condes", {nullptr}},      //
        // {"constructor", {nullptr}}, //
        // {"define", {nullptr}},      //
        // {"error", {nullptr}},       //
        // {"export", {nullptr}},      //
        // {"exportzp", {nullptr}},    //
        // {"fileopt", {nullptr}},     //
        // {"fopt", {nullptr}},        //
        // {"forceimport", {nullptr}}, //
        // {"global", {nullptr}},      //
        // {"hibytes", {nullptr}},     //
        // {"import", {nullptr}},      //
        // {"importzp", {nullptr}},    //
        // {"include", {nullptr}},     //
        // {"interruptor", {nullptr}}, //
        // {"lobytes", {nullptr}},     //
        // {"local", {nullptr}},       //
        // {"out", {nullptr}},         //
        // {"reloc", {nullptr}},       //
        // {"res", {nullptr}},         //
        // {"setcpu", {nullptr}},      //
        // {"tag", {nullptr}},         //
        // {"warning", {nullptr}},     //

        // {"enum", {nullptr}},    //
        // {"endenum", {nullptr}}, //
        // {"proc", {nullptr}},    //
        // {"endproc", {nullptr}}, //
        // {"scope", {nullptr}},    //
        // {"endscope", {nullptr}}, //
        // {"struct", {nullptr}},    //
        // {"endstruct", {nullptr}}, //

        // {"bss", {nullptr}}, //
        // {"segment", {nullptr}}, //
        // {"code", {nullptr}}, //
        // {"data", {nullptr}}, //
        // {"rodata", {nullptr}}, //
        // {"zeropage", {nullptr}}, //

        //extensions
        {"isr", {&CompilationContext::ParseIsrCommand}},   //
        {"text", {&CompilationContext::ParseTextCommand}}, //
        // {"symbol", {&CompilationContext::ParseSymbolCommand}}, //
};

const std::unordered_map<std::string, Address_t> CompilationContext::kIsrMap = {
    {"reset", kResetVector},
    {"irq", kIrqVector},
    {"nmib", kNmibVector},
};

void CompilationContext::HandleCommand(const Token &command_token,
                                       LineTokenizer &line_tokenizer) {
    auto command = command_token.View();
    command.remove_prefix(1);
    if (auto it = kCommandParseInfo.find(std::string(command));
        it == kCommandParseInfo.end()) {
        ThrowCompilationError(CompilationError::UnknownCommand, command_token);
    } else {
        auto handler = it->second.handler;
        (this->*handler)(line_tokenizer);
    }
}

void CompilationContext::ParseDataCommand(LineTokenizer &tokenizer,
                                          Address_t element_size) {
    for (auto tok : tokenizer.TokenList(",")) {
        switch (GetTokenType(tok, program)) {
        case TokenType::kAlias:
        case TokenType::kValue:
            EmitBytes(ParseTokenToBytes(tok, element_size));
            continue;

        case TokenType::kUnknown:
        case TokenType::kSymbol:
            if (element_size == 2) {
                PutSymbolReference(RelocationMode::Absolute, tok.String(),
                                   current_position);
                continue;
            } else {
                ThrowCompilationError(CompilationError::InvalidOperandSize, tok,
                                      "Cannot put 1 byte reference to symbol");
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

void CompilationContext::ParseAlignCommand(LineTokenizer &tokenizer) {
    auto tok = tokenizer.NextToken();
    Address_t alignment = 0;
    if (tok == "page") {
        alignment = kMemoryPageSize;
    } else {
        try {
            alignment = ParseWord(tok.View());
        } catch (...) {
            ThrowCompilationError(CompilationError::InvalidCommandArgument, tok,
                                  "Cannot parse value");
        }
    }

    if (std::popcount(alignment) != 1) {
        ThrowCompilationError(CompilationError::InvalidCommandArgument, tok,
                              "Invalid alignment value");
    }
    auto mask = alignment - 1;

    auto new_address = current_position;
    if ((new_address & mask) != 0) {
        new_address &= ~mask;
        new_address += alignment;
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
    case TokenType::kSymbol:
        ThrowCompilationError(CompilationError::SymbolIsNotAllowed, tok);
    }

    ThrowCompilationError(CompilationError::InvalidToken, tok);
}

void CompilationContext::ParseIsrCommand(LineTokenizer &tokenizer) {
    auto isr = tokenizer.NextToken();

    Address_t addr{};
    try {
        addr = kIsrMap.at(isr.String());
    } catch (const std::out_of_range &) {
        ThrowCompilationError(CompilationError::UnknownIsr, isr);
    }

    auto tok = tokenizer.NextToken();
    Log("Setting ISR {} to '{}'", isr.value, tok.value);

    switch (GetTokenType(tok, program)) {
    case TokenType::kAlias:
    case TokenType::kValue:
        break;

    case TokenType::kUnknown:
    case TokenType::kSymbol:
        PutSymbolReference(RelocationMode::Absolute, tok.String(), addr);
        return;
    }

    ThrowCompilationError(CompilationError::InvalidIsrArgument, tok,
                          "Unknown isr command argument '{}'", tok.String());
}

void CompilationContext::ParseSymbolCommand(LineTokenizer &tokenizer) {
}

std::vector<uint8_t>
CompilationContext::ParseTokenToBytes(const Token &value_token,
                                      size_t expected_byte_size) const {
    try {
        return ParseImmediateValue(value_token.View(), program.aliases,
                                   expected_byte_size);
    } catch (const std::exception &e) {
        ThrowCompilationError(CompilationError::InvalidToken, value_token, e.what());
    }
}

void CompilationContext::BeginSymbol(const Token &name_token) {
    auto view = name_token.View();
    if (view.ends_with(":")) {
        view.remove_suffix(1);
    }
    auto symbol_name = std::string(view);

    if (auto symbol = program.FindSymbol(symbol_name); symbol == nullptr) {
        Log("Adding symbol '{}' at {:04x}", symbol_name, current_position);
        auto l = SymbolInfo{
            .name = symbol_name,
            .offset = current_position,
            .imported = false,
        };
        program.AddSymbol(std::make_shared<SymbolInfo>(l));
    } else {
        Log("Found symbol '{}' at {:04x}", symbol_name, current_position);
        symbol->imported = false;
        if (HasValue(symbol->offset)) {
            ThrowCompilationError(CompilationError::SymbolRedefinition, name_token);
        }

        symbol->offset = current_position;
    }
}

void CompilationContext::EmitInstruction(LineTokenizer &tokenizer,
                                         const InstructionParsingInfo &instruction) {
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

    Token meta_token{first_token.location, token};
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

    std::set_intersection(
        argument.possible_address_modes.begin(), argument.possible_address_modes.end(), //
        instruction_address_modes.begin(), instruction_address_modes.end(),             //
        std::inserter(selector.possible_address_modes,
                      selector.possible_address_modes.begin()));

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
            PutSymbolReference(*r.relocation_mode, r.relocation_symbol,
                               r.relocation_position);
        }
    } catch (const std::exception &e) {
        ThrowCompilationError(CompilationError::InvalidOperandArgument, first_token, "{}",
                              e.what());
    }
}

void CompilationContext::PutSymbolReference(RelocationMode mode,
                                            const std::string &symbol,
                                            Address_t position) {
    auto relocation = std::make_shared<RelocationInfo>();
    std::shared_ptr<SymbolInfo> symbol_ptr = program.FindSymbol(symbol);
    if (symbol_ptr == nullptr) {
        Log("Adding reference at {:04x} to unknown symbol '{}'", position, symbol);
        symbol_ptr = program.AddSymbol(SymbolInfo{
            .name = symbol,
            .imported = true,
        });
    } else {
        Log("Adding reference at {:04x} to symbol '{}'", position, symbol);
    }

    relocation->position = position;
    relocation->mode = mode;
    relocation->target_symbol = symbol_ptr;
    program.relocations.insert(relocation);
}

void CompilationContext::AddDefinition(const Token &name_token,
                                       const Token &value_token) {
    auto data = ParsePackedIntegral(value_token.View());
    Log("Adding definition '{}' = '{}'", name_token.String(), ToHex(data, ""));
    if (program.FindAlias(name_token.String()) != nullptr) {
        ThrowCompilationError(CompilationError::AliasRedefinition, name_token);
    }
    program.AddAlias(std::make_shared<ValueAlias>(ValueAlias{name_token.String(), data}));
}

void CompilationContext::EmitBytes(const ByteVector &data) {
    program.sparse_binary_code.PutBytes(current_position, data);
    current_position += static_cast<Address_t>(data.size());
}

void CompilationContext::UpdateRelocations() {
    for (const auto &relocation : program.relocations) {
        auto symbol = relocation->target_symbol.lock();
        if (!symbol) {
            throw std::runtime_error("TODO (!symbol)");
        }
        Log("Relocating reference to symbol '{}' at {}", symbol->name,
            to_string(relocation));

        if (relocation->mode == RelocationMode::Absolute) {
            auto bytes = ToBytes(symbol->offset, std::nullopt);
            program.sparse_binary_code.PutBytes(relocation->position, bytes, true);
        } else {
            auto jump = RelativeJumpOffset(relocation->position + 1,
                                           GetOr(symbol->offset, relocation->position));
            program.sparse_binary_code.PutBytes(relocation->position, ToBytes(jump),
                                                true);
        }
    }
}

void CompilationContext::AddDefinition(const SymbolDefinition &symbol) {
    if (!symbol.segment.has_value()) {
        program.AddAlias(ValueAlias{
            .name = symbol.name,
            .value = ToBytes(symbol.value, std::nullopt),
        });
    } else {
        program.AddSymbol(SymbolInfo{
            .name = symbol.name,
            .offset = symbol.value,
            .segment = symbol.segment,
            .imported = true,
        });
    }
}

} // namespace emu::emu6502::assembler
