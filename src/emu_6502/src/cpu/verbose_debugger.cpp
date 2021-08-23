#include "emu_6502/cpu/verbose_debugger.hpp"
#include <boost/algorithm/string/join.hpp>

namespace emu::emu6502::cpu {

namespace {

std::string FormatAddressMode(AddressMode addres_mode, const Registers &regs,
                              std::optional<uint8_t> byte_low,
                              std::optional<uint8_t> byte_hi) {
    switch (addres_mode) {
    case AddressMode::Immediate:
        return fmt::format("#${:02x}", byte_low.value());
    case AddressMode::Implied:
        return "";
    case AddressMode::ABS:
        return fmt::format("${:02x}{:02x}", byte_hi.value(), byte_low.value());
    case AddressMode::ZP:
        return fmt::format("${:02x}", byte_low.value());
    case AddressMode::ZPX:
        return fmt::format("${:02x},X", byte_low.value());
    case AddressMode::ZPY:
        return fmt::format("${:02x},Y", byte_low.value());
    case AddressMode::ABSX:
        return fmt::format("${:02x}{:02x},X", byte_hi.value(), byte_low.value());
    case AddressMode::ABSY:
        return fmt::format("${:02x}{:02x},Y", byte_hi.value(), byte_low.value());
    case AddressMode::INDX:
        return fmt::format("(${:02x},X)", byte_low.value());
    case AddressMode::INDY:
        return fmt::format("(${:02x}),Y", byte_low.value());
    case AddressMode::ACC:
        return "A";
    case AddressMode::REL: {
        auto s8 = static_cast<int8_t>(byte_low.value());
        return fmt::format("{} (${:04x})", s8, regs.program_counter + 2 + s8);
    }
    case AddressMode::ABS_IND:
        return fmt::format("(${:02x}{:02x})", byte_hi.value(), byte_low.value());
    }

    return "?";
}

} // namespace

VerboseDebugger::VerboseDebugger(InstructionSet instruction_set, Memory16 *memory,
                                 Clock *clock, std::ostream *verbose_stream)
    : memory(memory), clock(clock), verbose_stream(verbose_stream) {

    known_opcodes.fill(nullptr);
    for (auto &[opcode, info] : GetInstructionSet(instruction_set)) {
        known_opcodes.at(opcode) = &info;
    }
}

void VerboseDebugger::OnNextInstruction(const Registers &regs) {
    if (verbose_stream == nullptr) {
        return;
    }

    std::string debug_line = fmt::format("{:016x} | {} | {:04x}: ", clock->CurrentCycle(),
                                         regs.Dump(), regs.program_counter);

    auto opcode = memory->DebugRead(regs.program_counter);
    if (!opcode.has_value()) {
        debug_line += "?";
    } else {
        auto *opcode_info = known_opcodes.at(opcode.value());
        std::array<std::string, 4> consumed_bytes;
        consumed_bytes.fill("  ");
        consumed_bytes[0] = fmt::format("{:02x}", opcode.value());

        std::string assembly_code = "?";
        if (opcode_info != nullptr) {
            assembly_code = opcode_info->mnemonic;
            assembly_code += " ";
            std::optional<uint8_t> byte_low;
            std::optional<uint8_t> byte_hi;

            auto size = ArgumentByteSize(opcode_info->addres_mode);
            switch (size) {
            case 2:
                byte_hi = memory->DebugRead(regs.program_counter + 2);
                [[fallthrough]];
            case 1:
                byte_low = memory->DebugRead(regs.program_counter + 1);
                [[fallthrough]];
            case 0:
                break;
            default:
                throw std::runtime_error(fmt::format("Invalid operand size {}", size));
            }

            if (byte_low.has_value()) {
                consumed_bytes[1] = fmt::format("{:02x}", byte_low.value());
            }
            if (byte_hi.has_value()) {
                consumed_bytes[2] = fmt::format("{:02x}", byte_hi.value());
            }
            auto mem = memory->DebugReadRange(regs.program_counter, 2);

            assembly_code +=
                FormatAddressMode(opcode_info->addres_mode, regs, byte_low, byte_hi);
        }

        debug_line +=
            fmt::format("{} | {}", boost::join(consumed_bytes, " "), assembly_code);
    }
    debug_line += "\n";
    *verbose_stream << debug_line;
}

} // namespace emu::emu6502::cpu
