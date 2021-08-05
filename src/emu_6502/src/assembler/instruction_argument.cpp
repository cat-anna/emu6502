#include "instruction_argument.hpp"
#include "emu_6502/assembler/compilation_error.hpp"
#include "emu_core/byte_utils.hpp"
#include "emu_core/text_utils.hpp"
#include <charconv>
#include <cinttypes>
#include <fmt/format.h>
#include <limits>
#include <regex>
#include <stdexcept>
#include <string_view>

namespace emu::emu6502::assembler {

std::set<AddressMode> FilterPossibleModes(const std::set<AddressMode> &modes,
                                          size_t size) {
    std::set<AddressMode> r;
    for (auto m : modes) {
        if (ArgumentByteSize(m) == size) {
            r.emplace(m);
        }
    }
    return r;
}

ByteVector ParseImmediateValue(std::string_view data, const AliasMap &aliases,
                               std::optional<size_t> expected_size) {
    auto check = [&](ByteVector v) {
        if (expected_size.has_value() && v.size() != expected_size.value()) {
            throw std::runtime_error(
                fmt::format("Expected '{}' to have size {}, but got {}", data,
                            expected_size.value(), v.size()));
        }
        return v;
    };

    if (!data.starts_with("$")) {
        if (const auto it = aliases.find(std::string(data)); it != aliases.end()) {
            return check(it->second->value);
        }

        if (data.starts_with("\"")) {
            data.remove_prefix(1);
            data.remove_suffix(1);
            return check(ToBytes(data));
        }
    }

    return ParsePackedIntegral(data, expected_size);
}

ByteVector ParseTextValue(const Token &token, bool include_trailing_zero) {
    auto view = token.View();

    if (view.starts_with("\"")) {
        view.remove_prefix(1);
        view.remove_suffix(1);
    }

    auto r = ToBytes(view);
    if (include_trailing_zero) {
        r.push_back(0);
    }
    return r;
}

std::string to_string(const InstructionArgument &ia) {
    std::string r = "InstructionArgument{{";
    for (auto i : ia.possible_address_modes) {
        r += to_string(i);
        r += ",";
    }
    r += "}, ";

    switch (ia.argument_value.index()) {
    case 0:
        r += "NULL";
        break;
    case 1:
        r += "'" + std::get<1>(ia.argument_value) + "'";
        break;
    case 2:
        r += "{";
        for (auto i : std::get<2>(ia.argument_value)) {
            r += fmt::format("{:02x},", i);
        }
        r += "}";
        break;

    default:
        r += "?";
    }

    r += "}";
    return r;
}

InstructionArgument ParseInstructionArgument(const Token &token,
                                             const AliasMap &aliases) {
    // +---------------------+--------------------------+
    // |      mode           |     assembler format     |
    // +=====================+==========================+

    // | Implied             |                          |
    if (!token) {
        return InstructionArgument{
            .possible_address_modes = {AddressMode::Implied},
            .argument_value = nullptr,
        };
    }

    // | Accumulator         |          A               |
    if (token.View() == "A") {
        return InstructionArgument{
            .possible_address_modes = {AddressMode::ACC},
            .argument_value = nullptr,
        };
    }

    using AM = AddressMode;
    const std::vector<std::tuple<std::regex, std::set<AM>>> fmt_regex = {
        // | Immediate           |          #aa             |
        {std::regex{R"==(^#([$\w]+)$)=="}, {AM::Immediate}},

        // | Absolute            |          aaaa            |
        // | Zero Page           |          aa              |
        // | Relative            |          aaaa            |
        {std::regex{R"==(^([$\w]+)$)=="}, {AM::ABS, AM::ZP, AM::REL}},

        // | Indirect Absolute   |          (aaaa)          |
        {std::regex{R"==(^\(([$\w]+)\)$)=="}, {AM::ABS_IND}},

        // | Zero Page Indexed,X |          aa,X            |
        // | Absolute Indexed,X  |          aaaa,X          |
        {std::regex{R"==(^([$\w]+),X$)=="}, {AM::ABSX, AM::ZPX}},

        // | Zero Page Indexed,Y |          aa,Y            |
        // | Absolute Indexed,Y  |          aaaa,Y          |
        {std::regex{R"==(^([$\w]+),Y$)=="}, {AM::ABSY, AM::ZPY}},

        // | Indexed Indirect    |          (aa,X)          |
        {std::regex{R"==(^\(([$\w]+),X\)$)=="}, {AM::INDX}},

        // | Indirect Indexed    |          (aa),Y          |
        {std::regex{R"==(^\(([$\w]+)\),Y$)=="}, {AM::INDY}},
    };

    for (const auto &[regex, matched_modes] : fmt_regex) {
        std::smatch match;
        auto str = token.String();
        if (std::regex_match(str, match, regex)) {
            std::string s_value = match[1];
            auto value = std::string_view(s_value);

            if (value.empty()) {
                throw std::runtime_error("Failed to match operand argument with regex");
            }

            InstructionArgument ia;

            if (value.starts_with("$")) {
                std::vector<uint8_t> data = ParseImmediateValue(value, aliases);
                auto possible_address_modes = matched_modes;
                possible_address_modes.erase(AM::REL);
                ia = InstructionArgument{
                    .possible_address_modes =
                        FilterPossibleModes(possible_address_modes, data.size()),
                    .argument_value = data,
                };
            } else {
                if (auto it = aliases.find(s_value); it != aliases.end()) {
                    auto possible_address_modes = matched_modes;
                    possible_address_modes.erase(AM::REL);
                    const auto &v = it->second->value;
                    ia = InstructionArgument{
                        .possible_address_modes =
                            FilterPossibleModes(possible_address_modes, v.size()),
                        .argument_value = v,
                    };
                } else {
                    ia = InstructionArgument{
                        .possible_address_modes = matched_modes,
                        .argument_value = s_value,
                    };
                }
            }

            if (ia.possible_address_modes.empty()) {
                ThrowCompilationError(CompilationError::InvalidOperandArgument, token);
            }
            return ia;
        }
    }

    ThrowCompilationError(CompilationError::InvalidOperandArgument, token);
}

std::string to_string(TokenType tt) {
    switch (tt) {
    case TokenType::kValue:
        return "TokenType::kValue";
    case TokenType::kSymbol:
        return "TokenType::kSymbol";
    case TokenType::kAlias:
        return "TokenType::kAlias";
    case TokenType::kUnknown:
        return "TokenType::kUnknown";
    }
    return fmt::format("TokenType::{}", static_cast<int>(tt));
}

std::ostream &operator<<(std::ostream &o, TokenType tt) {
    return o << to_string(tt);
}

TokenType GetTokenType(const Token &value_token, const Program &program) {
    return GetTokenType(value_token, &program.aliases, &program.symbols);
}

TokenType GetTokenType(const Token &value_token, const AliasMap *aliases,
                       const SymbolMap *symbols) {
    auto sv = value_token.View();
    if (sv.starts_with("0x") || sv.starts_with("0X") || sv.starts_with("\"") ||
        sv.starts_with("$")) {
        return TokenType::kValue;
    }

    if (VerifyString(sv, [](auto i) { return isdigit(i) != 0; })) {
        return TokenType::kValue;
    }

    auto text = std::string(sv);
    if (aliases != nullptr && aliases->contains(text)) {
        return TokenType::kAlias;
    }

    if (symbols != nullptr && symbols->contains(text)) {
        return TokenType::kSymbol;
    }

    return TokenType::kUnknown;
}

} // namespace emu::emu6502::assembler
