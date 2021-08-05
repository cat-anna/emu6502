#pragma once

#include "emu_core/memory.hpp"
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
using Reg8 = uint8_t;
using Reg16 = uint16_t;
using MemPtr = uint16_t;

struct OpcodeInfo {
    Opcode opcode;
    std::string_view mnemonic;
    AddressMode addres_mode;
};

using OpcodeInstructionMap = std::unordered_map<Opcode, OpcodeInfo>;

const OpcodeInstructionMap &Get6502InstructionSet();
const OpcodeInstructionMap &Get6502EmuInstructionSet();
const OpcodeInstructionMap &GetInstructionSet(InstructionSet instruction_set);

using MemPtr = Memory16::Address_t;

constexpr MemPtr kIrqVector = 0xFFFE;
constexpr MemPtr kResetVector = 0xFFFC;
constexpr MemPtr kNmibVector = 0xFFFA;

constexpr MemPtr kStackBase = 0x0100;
constexpr MemPtr kZeroPageBase = 0x0000;

constexpr Reg8 kStackResetValue = 0xFF;
constexpr Reg8 kNegativeBit = 0x80;

constexpr MemPtr kMemoryPageSize = 0x0100;

enum class Interrupt : Reg8 {
    None = 0,
    Nmi,
    Irq,
    Brk,
};

std::string to_string(Interrupt interrupt);
MemPtr InterruptHandlerAddress(Interrupt interrupt);

} // namespace emu::emu6502
