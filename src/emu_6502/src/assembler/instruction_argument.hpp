#pragma once

#include "emu_6502/assembler/tokenizer.hpp"
#include "emu_6502/instruction_set.hpp"
#include "emu_core/program.hpp"
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace emu::emu6502::assembler {

std::set<AddressMode> FilterPossibleModes(const std::set<AddressMode> &modes,
                                          size_t size);

using ArgumentValueVariant =
    std::variant<std::nullptr_t, std::string, std::vector<uint8_t>>;
struct InstructionArgument {
    std::set<AddressMode> possible_address_modes;
    ArgumentValueVariant argument_value;

    bool operator==(const InstructionArgument &other) const {
        return possible_address_modes == other.possible_address_modes && //
               argument_value == other.argument_value;
    }
};

std::string to_string(const InstructionArgument &ia);

InstructionArgument ParseInstructionArgument(const Token &token, const AliasMap &aliases);

ByteVector ParseImmediateValue(std::string_view data, const AliasMap &aliases,
                               std::optional<size_t> expected_size = std::nullopt);
ByteVector ParseTextValue(const Token &token, bool include_trailing_zero);

enum class TokenType {
    kValue = 1,
    kSymbol,
    kAlias,
    kUnknown,
};

std::string to_string(TokenType tt);
std::ostream &operator<<(std::ostream &o, TokenType tt);

TokenType GetTokenType(const Token &value_token, const AliasMap *aliases,
                       const SymbolMap *symbols);
TokenType GetTokenType(const Token &value_token, const Program &program);

} // namespace emu::emu6502::assembler
