#include "instruction_argument.hpp"
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

namespace {
void FilterPossibleModes(std::set<AddressMode> &modes, size_t size) {
    using AM = AddressMode;
    if (size == 1) {
        modes.erase(AM::ABSX);
        modes.erase(AM::ABSY);
        modes.erase(AM::ABS);
    } else {
        modes.erase(AM::ZPX);
        modes.erase(AM::ZPY);
        modes.erase(AM::ZP);
    }

    if (modes.empty()) {
        throw std::runtime_error("Impossible operand size");
    }

    for (auto i : modes) {
        if (ArgumentByteSize(i) != size) {
            throw std::runtime_error("Invalid operand size");
        }
    }
}
} // namespace

ByteVector ParseImmediateValue(std::string_view data, const AliasMap &aliases, std::optional<size_t> expected_size) {
    auto check = [&](ByteVector v) {
        if (expected_size.has_value() && v.size() != expected_size.value()) {
            throw std::runtime_error(
                fmt::format("Expected '{}' to have size {}, but got {}", data, expected_size.value(), v.size()));
        }
        return v;
    };

    if (!data.starts_with("$")) {
        if (const auto it = aliases.find(std::string(data)); it != aliases.end()) {
            ByteVector r;
            const auto &v = it->second->value;
            if (expected_size.has_value() && expected_size.value() > v.size()) {
                r.resize(expected_size.value() - v.size(), 0);
            }
            std::copy(v.begin(), v.end(), std::back_inserter(r));
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

InstructionArgument ParseInstructionArgument(std::string_view arg, const AliasMap &aliases) {
    // +---------------------+--------------------------+
    // |      mode           |     assembler format     |
    // +=====================+==========================+

    // | Implied             |                          |
    if (arg.empty()) {
        return InstructionArgument{
            .possible_address_modes = {AddressMode::Implied},
            .argument_value = nullptr,
        };
    }

    // | Accumulator         |          A               |
    if (arg == "A") {
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
        auto str = std::string(arg);
        if (std::regex_match(str, match, regex)) {
            std::string s_value = match[1];
            auto value = std::string_view(s_value);

            if (value.empty()) {
                //TODO:
                throw std::runtime_error("value.empty()");
            }

            auto possible_address_modes = matched_modes;
            if (value.starts_with("$")) {
                std::vector<uint8_t> data = ParseImmediateValue(value, aliases);
                possible_address_modes.erase(AM::REL);
                FilterPossibleModes(possible_address_modes, data.size());
                return InstructionArgument{
                    .possible_address_modes = {possible_address_modes.begin(), possible_address_modes.end()},
                    .argument_value = data,
                };
            } else {
                if (auto it = aliases.find(s_value); it != aliases.end()) {
                    possible_address_modes.erase(AM::REL);
                    const auto &v = it->second->value;
                    FilterPossibleModes(possible_address_modes, v.size());
                    return InstructionArgument{
                        .possible_address_modes = {possible_address_modes.begin(), possible_address_modes.end()},
                        .argument_value = v,
                    };
                } else {
                    return InstructionArgument{
                        .possible_address_modes = {possible_address_modes.begin(), possible_address_modes.end()},
                        .argument_value = s_value,
                    };
                }
            }
        }
    }

    throw std::runtime_error("not implemted");
}

std::string to_string(TokenType tt) {
    switch (tt) {
    case TokenType::kValue:
        return "TokenType::kValue";
    case TokenType::kLabel:
        return "TokenType::kLabel";
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
    return GetTokenType(value_token, &program.aliases, &program.labels);
}

TokenType GetTokenType(const Token &value_token, const AliasMap *aliases, const LabelMap *labels) {
    auto sv = value_token.View();
    if (sv.starts_with("0x") || sv.starts_with("0X") || sv.starts_with("\"") || sv.starts_with("$")) {
        return TokenType::kValue;
    }

    if (VerifyString(sv, [](auto i) { return isdigit(i) != 0; })) {
        return TokenType::kValue;
    }

    auto text = std::string(sv);
    if (aliases != nullptr && aliases->contains(text)) {
        return TokenType::kAlias;
    }

    if (labels != nullptr && labels->contains(text)) {
        return TokenType::kLabel;
    }

    return TokenType::kUnknown;
}

} // namespace emu::emu6502::assembler
