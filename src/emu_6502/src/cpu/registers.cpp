#include "emu_6502/cpu/registers.hpp"
#include <fmt/format.h>

namespace emu::emu6502::cpu {

//-----------------------------------------------------------------------------

std::string Registers::DumpFlags() const {
    std::string r;
    r += fmt::format("{:02x}[", flags);
    r += TestFlag(Flags::Negative) ? "N" : "-";
    r += TestFlag(Flags::Overflow) ? "V" : "-";
    r += TestFlag(Flags::NotUsed) ? "?" : "-";
    r += TestFlag(Flags::Brk) ? "B" : "-";
    r += TestFlag(Flags::DecimalMode) ? "D" : "-";
    r += TestFlag(Flags::IRQB) ? "I" : "-";
    r += TestFlag(Flags::Zero) ? "Z" : "-";
    r += TestFlag(Flags::Carry) ? "C" : "-";
    r += "]";
    return r;
}

std::string Registers::Dump() const {
    std::string r;
    r += fmt::format("pc:{:04x}", program_counter);
    r += fmt::format(" s:{:02x}", stack_pointer);
    r += fmt::format(" a:{:02x}", a);
    r += fmt::format(" x:{:02x}", x);
    r += fmt::format(" y:{:02x}", y);
    r += " f:";
    r += DumpFlags();
    return r;
}

void Registers::Reset() {
    program_counter = 0;
    a = x = y = 0;
    stack_pointer = kStackResetValue;
    flags = 0;
}

} // namespace emu::emu6502::cpu