#pragma once

#include <cstdint>
#include <string_view>
#include <unordered_map>

namespace emu::emu6502 {

enum class InstructionSet {
    Unknown,

    NMOS6502,
    NMOS6502Emu,

    Default = NMOS6502,
};

enum class AddressMode {
    Immediate,
    Implied,
    ABS,
    ZP,
    ZPX,
    ZPY,
    ABSX,
    ABSY,
    INDX,
    INDY,
    ACC,
    REL,
    ABS_IND,

    IM = Immediate,
};

std::string to_string(AddressMode mode);
size_t ArgumentByteSize(AddressMode mode);

using Opcode = uint8_t;

struct OpcodeInfo {
    Opcode opcode;
    std::string_view mnemonic;
    AddressMode addres_mode;
    // uint8_t execution_cycles; //TODO
};

using OpcodeInstructionMap = std::unordered_map<Opcode, OpcodeInfo>;

const OpcodeInstructionMap &Get6502InstructionSet();
const OpcodeInstructionMap &Get6502EmuInstructionSet();
const OpcodeInstructionMap &GetInstructionSet(InstructionSet instruction_set);

} // namespace emu::emu6502
