#pragma once

#include "cpu6502.hpp"
#include <emu_core/clock.hpp>
#include <emu_core/memory.hpp>

namespace emu::cpu6502::instructions {

uint8_t FetchNextByte(Cpu6502 *cpu) { // mode #
    return cpu->memory->Load(cpu->reg.program_counter++);
}

MemPtr GetAbsoluteAddress(Cpu6502 *cpu) { // mode: a
    MemPtr addr = FetchNextByte(cpu);
    return addr | FetchNextByte(cpu) << 8;
}

MemPtr GetAddressAbsoluteIndexedIndirectWithX(Cpu6502 *cpu) { // mode (a,x)
    MemPtr location = GetAbsoluteAddress(cpu);
    location += cpu->reg.x;
    cpu->clock->WaitForNextCycle();
    MemPtr addr = cpu->memory->Load(location++);
    return addr | cpu->memory->Load(location) << 8;
}

MemPtr GetAddressAbsoluteIndexedWithX(Cpu6502 *cpu) { // mode a,x
    auto r = GetAbsoluteAddress(cpu);
    cpu->clock->WaitForNextCycle();
    return r + cpu->reg.x;
}

MemPtr GetAddressAbsoluteIndexedWithY(Cpu6502 *cpu) { // mode a,y
    auto r = GetAbsoluteAddress(cpu);
    cpu->clock->WaitForNextCycle();
    return r + cpu->reg.y;
}

MemPtr GetAddressAbsoluteIndirect(Cpu6502 *cpu) { // mode (a)
    auto location = GetAbsoluteAddress(cpu);
    MemPtr addr = cpu->memory->Load(location++);
    return addr | cpu->memory->Load(location) << 8;
}

MemPtr GetAddressAccumulator(Cpu6502 *cpu) { // mode A
    return cpu->reg.a;
}

MemPtr GetAddressProgramCounterRelative(Cpu6502 *cpu) { // mode r
    auto offset = FetchNextByte(cpu);
    return cpu->reg.program_counter + offset;
}

MemPtr GetStackAddress(Cpu6502 *cpu) { // mode s
    return cpu->reg.StackPointerMemoryAddress();
}

MemPtr GetZeroPageAddress(Cpu6502 *cpu) { //mode zp
    return FetchNextByte(cpu);
}

MemPtr GetAddresZeroPageIndexedIndirectWithX(Cpu6502 *cpu) { // mode (zp,x)
    return cpu->memory->Load(cpu->reg.x + GetZeroPageAddress(cpu));
}

MemPtr GetZeroPageIndirectAddressWithX(Cpu6502 *cpu) { // mode zp,x
    return GetZeroPageAddress(cpu) + cpu->reg.x;
}

MemPtr GetZeroPageIndirectAddressWithY(Cpu6502 *cpu) { // mode zp,y
    return GetZeroPageAddress(cpu) + cpu->reg.y;
}

MemPtr GetZeroPageIndirectAddress(Cpu6502 *cpu) { // mode (zp)
    return cpu->memory->Load(GetZeroPageAddress(cpu));
}

MemPtr GetAddresZeroPageIndirectIndexedWithY(Cpu6502 *cpu) { // mode (zp),y
    return cpu->memory->Load(GetZeroPageAddress(cpu)) + cpu->reg.y;
}

constexpr auto kAddressZP = &GetZeroPageAddress;
constexpr auto kAddressZPX = &GetZeroPageIndirectAddressWithX;
constexpr auto kAddressZPY = &GetZeroPageIndirectAddressWithY;
constexpr auto kAddressABS = &GetAbsoluteAddress;
constexpr auto kAddressABSX = &GetAddressAbsoluteIndexedWithX;
constexpr auto kAddressABSY = &GetAddressAbsoluteIndexedWithY;
constexpr auto kAddressINDX = &GetAddresZeroPageIndexedIndirectWithX;
constexpr auto kAddressINDY = &GetAddresZeroPageIndirectIndexedWithY;

} // namespace emu::cpu6502::instructions
