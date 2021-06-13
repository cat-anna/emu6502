#include "instrution_variant_compiler.hpp"
#include "emu_6502/assembler/compilation_error.hpp"

namespace emu::emu6502::assembler {

AddressMode InstructionVariantSelector::DispatchSelect(const ArgumentValueVariant &arg_variant) const {
    if (possible_address_modes.empty()) {
        ThrowCompilationError(CompilationError::InvalidOperandArgument, token);
    }

    auto modes = std::visit([&](auto &arg_value) { return Select(arg_value); }, arg_variant);

    if (modes.size() != 1) {
        ThrowCompilationError(CompilationError::InvalidOperandSize, token);
    }
    return *modes.begin();
}

std::set<AddressMode> InstructionVariantSelector::Select(std::nullptr_t) const {
    if (!possible_address_modes.contains(AddressMode::Implied)) {
        ThrowCompilationError(CompilationError::ImpliedModeNotSupported, token);
    }
    return possible_address_modes;
}

std::set<AddressMode> InstructionVariantSelector::Select(const std::string &label) const {
    if (const auto it = aliases.find(label); it != aliases.end()) {
        return FilterPossibleModes(possible_address_modes, it->second->value.size());
    } else {
        if (possible_address_modes.contains(AddressMode::REL)) {
            return possible_address_modes;
        } else {
            return FilterPossibleModes(possible_address_modes, RelocationSize(RelocationMode::Absolute));
        }
    }
}

std::set<AddressMode> InstructionVariantSelector::Select(const ByteVector &bv) const {
    return FilterPossibleModes(possible_address_modes, bv.size());
}

//-----------------------------------------------------------------------------

using Result = InstructionArgumentDataProcessor::Result;

Result InstructionArgumentDataProcessor::DispatchProcess(const ArgumentValueVariant &arg_variant) const {
    return std::visit([&](auto &arg_value) { return Process(arg_value); }, arg_variant);
}

Result InstructionArgumentDataProcessor::Process(std::nullptr_t) const {
    return Result{
        .bytes = {opcode.opcode},
    };
}

Result InstructionArgumentDataProcessor::Process(const std::vector<uint8_t> &data) const {
    if (ArgumentByteSize(opcode.addres_mode) < data.size()) {
        ThrowCompilationError(CompilationError::InvalidOperandSize, token);
    }

    ByteVector r{opcode.opcode};
    r.insert(r.end(), data.begin(), data.end());
    return Result{
        .bytes = r,
    };
}

Result InstructionArgumentDataProcessor::Process(const std::string &label) const {
    ByteVector r{opcode.opcode};
    auto mode = opcode.addres_mode == AddressMode::REL ? RelocationMode::Relative : RelocationMode::Absolute;
    r.resize(1 + RelocationSize(mode), 0);
    return Result{
        .bytes = r,
        .relocation_mode = mode,
        .relocation_position = current_position + 1u,
        .relocation_label = label,
    };
}

} // namespace emu::emu6502::assembler
