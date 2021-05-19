#pragma once

#include "cpu_6502/instruction_set.hpp"
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace emu::assembler6502 {

struct InstructionArgument {
    using AddressMode = cpu6502::AddressMode;
    std::vector<AddressMode> possible_address_modes;
    std::variant<std::nullptr_t, std::string, std::vector<uint8_t>> argument_value;

    bool operator==(const InstructionArgument &other) const {
        return possible_address_modes == other.possible_address_modes && argument_value == other.argument_value;
    }
};

std::string to_string(const InstructionArgument &ia);

InstructionArgument ParseInstructionArgument(std::string_view arg);

} // namespace emu::assembler6502
