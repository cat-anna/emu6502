#pragma once

#include <cpu/opcode.hpp>
#include <fmt/format.h>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace emu::assembler {

std::vector<std::string_view> Tokenize(std::string_view line);
uint8_t ParseByte(std::string_view text, int base = 0);
uint16_t ParseWord(std::string_view text, int base = 0);

inline std::vector<uint8_t> ToBytes(uint8_t v) {
    return {v};
}
inline std::vector<uint8_t> ToBytes(int8_t v) {
    return {static_cast<uint8_t>(v)};
}

inline std::vector<uint8_t> ToBytes(uint16_t v) {
    return {
        static_cast<uint8_t>(v & 0xFF),
        static_cast<uint8_t>(v >> 8),
    };
}

struct InstructionArgument {
    using AddressMode = emu::cpu6502::opcode::AddressMode;
    std::vector<AddressMode> possible_address_modes;
    std::variant<std::nullptr_t, std::string, std::vector<uint8_t>> argument_value;

    bool operator==(const InstructionArgument &other) const {
        return possible_address_modes == other.possible_address_modes && argument_value == other.argument_value;
    }

    std::string to_string() const {
        std::string r = "InstructionArgument{{";
        for (auto i : possible_address_modes) {
            r += emu::cpu6502::opcode::to_string(i);
            r += ",";
        }
        r += "}, ";

        switch (argument_value.index()) {
        case 0:
            r += "NULL";
            break;
        case 1:
            r += "'" + std::get<1>(argument_value) + "'";
            break;
        case 2:
            r += "{";
            for (auto i : std::get<2>(argument_value)) {
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
};

InstructionArgument ParseInstructionArgument(std::string_view arg);

} // namespace emu::assembler
