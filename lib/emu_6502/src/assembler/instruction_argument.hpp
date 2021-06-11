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

struct InstructionArgument {
    std::vector<AddressMode> possible_address_modes;
    std::variant<std::nullptr_t, std::string, std::vector<uint8_t>> argument_value;

    bool operator==(const InstructionArgument &other) const {
        return possible_address_modes == other.possible_address_modes && argument_value == other.argument_value;
    }
};

std::string to_string(const InstructionArgument &ia);

InstructionArgument ParseInstructionArgument(std::string_view arg, const AliasMap &aliases);
ByteVector ParseImmediateValue(std::string_view data, const AliasMap &aliases,
                               std::optional<size_t> expected_size = std::nullopt);

enum class TokenType {
    kValue = 1,
    kLabel,
    kAlias,
    kUnknown,
};

std::string to_string(TokenType tt);
std::ostream &operator<<(std::ostream &o, TokenType tt);

TokenType GetTokenType(const Token &value_token, const AliasMap *aliases, const LabelMap *labels);
TokenType GetTokenType(const Token &value_token, const Program &program);

} // namespace emu::emu6502::assembler
