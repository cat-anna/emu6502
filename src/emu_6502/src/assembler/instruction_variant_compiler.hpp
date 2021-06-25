
#pragma once

#include "emu_6502/assembler/tokenizer.hpp"
#include "emu_6502/instruction_set.hpp"
#include "emu_core/program.hpp"
#include "instruction_argument.hpp"
#include <set>
#include <string_view>
#include <vector>

namespace emu::emu6502::assembler {

struct InstructionVariantSelector {
    std::set<AddressMode> possible_address_modes;
    const Token &token;
    const AliasMap &aliases;

    AddressMode DispatchSelect(const ArgumentValueVariant &arg_variant) const;

    std::set<AddressMode> Select(const std::string &symbol) const;
    std::set<AddressMode> Select(std::nullptr_t) const;
    std::set<AddressMode> Select(const ByteVector &bv) const;
};

struct InstructionArgumentDataProcessor {
    const OpcodeInfo &opcode;
    const Token &token;
    const Address_t current_position;

    struct Result {
        ByteVector bytes;

        std::optional<RelocationMode> relocation_mode = std::nullopt;
        Address_t relocation_position = 0;
        std::string relocation_symbol{};
    };

    Result DispatchProcess(const ArgumentValueVariant &arg_variant) const;

    Result Process(std::nullptr_t) const;
    Result Process(const ByteVector &data) const;
    Result Process(const std::string &symbol) const;
};

} // namespace emu::emu6502::assembler
