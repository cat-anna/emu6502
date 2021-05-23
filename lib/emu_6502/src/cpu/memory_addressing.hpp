#pragma once

#include "emu6502/cpu/cpu.hpp"
#include <emu_core/clock.hpp>
#include <emu_core/memory.hpp>

namespace emu::emu6502::cpu::instructions {

uint8_t FetchNextByte(Cpu *cpu) { // mode #
    return cpu->memory->Load(cpu->reg.program_counter++);
}

MemPtr GetAbsoluteAddress(Cpu *cpu) { // mode: a
    MemPtr addr = FetchNextByte(cpu);
    return addr | FetchNextByte(cpu) << 8;
}

MemPtr GetAddressAbsoluteIndexedIndirectWithX(Cpu *cpu) { // mode (a,x)
    MemPtr location = GetAbsoluteAddress(cpu);
    location += cpu->reg.x;
    cpu->WaitForNextCycle();
    MemPtr addr = cpu->memory->Load(location++);
    return addr | cpu->memory->Load(location) << 8;
}

template <bool fast>
MemPtr GetAddressAbsoluteIndexedWithX(Cpu *cpu) { // mode a,x
    auto r = GetAbsoluteAddress(cpu);
    if constexpr (!fast) {
        cpu->WaitForNextCycle(); //TODO: if across page ?
    }
    return r + cpu->reg.x;
}

template <bool fast>
MemPtr GetAddressAbsoluteIndexedWithY(Cpu *cpu) { // mode a,y
    auto r = GetAbsoluteAddress(cpu);
    if constexpr (!fast) {
        cpu->WaitForNextCycle(); //TODO: if across page ?
    }
    return r + cpu->reg.y;
}

MemPtr GetAddressAbsoluteIndirect(Cpu *cpu) { // mode (a)
    auto location = GetAbsoluteAddress(cpu);
    MemPtr addr = cpu->memory->Load(location++);
    return addr | cpu->memory->Load(location) << 8;
}

MemPtr GetAddressAccumulator(Cpu *cpu) { // mode A
    return cpu->reg.a;
}

MemPtr GetAddressProgramCounterRelative(Cpu *cpu) { // mode r
    auto offset = FetchNextByte(cpu);
    cpu->WaitForNextCycle();
    return cpu->reg.program_counter + offset;
}

MemPtr GetStackAddress(Cpu *cpu) { // mode s
    return cpu->reg.StackPointerMemoryAddress();
}

MemPtr GetZeroPageAddress(Cpu *cpu) { //mode zp
    return FetchNextByte(cpu);
}

MemPtr GetAddresZeroPageIndexedIndirectWithX(Cpu *cpu) { // mode (zp,x)
    MemPtr zp = GetZeroPageAddress(cpu);
    cpu->WaitForNextCycle();
    MemPtr indirect = zp + cpu->reg.x;
    cpu->WaitForNextCycle();
    return cpu->memory->Load(indirect);
}

MemPtr GetZeroPageIndirectAddressWithX(Cpu *cpu) { // mode zp,x
    MemPtr zp = GetZeroPageAddress(cpu);
    cpu->WaitForNextCycle();
    return zp + cpu->reg.x;
}

MemPtr GetZeroPageIndirectAddressWithY(Cpu *cpu) { // mode zp,y
    MemPtr zp = GetZeroPageAddress(cpu);
    cpu->WaitForNextCycle();
    return zp + cpu->reg.y;
}

MemPtr GetZeroPageIndirectAddress(Cpu *cpu) { // mode (zp)
    return cpu->memory->Load(GetZeroPageAddress(cpu));
}

template <bool fast>
MemPtr GetAddresZeroPageIndirectIndexedWithY(Cpu *cpu) { // mode (zp),y
    MemPtr zp = GetZeroPageAddress(cpu);
    cpu->WaitForNextCycle();
    auto mem = cpu->memory->Load(zp);
    if constexpr (!fast) {
        cpu->WaitForNextCycle();
    }
    return mem + cpu->reg.y;
}

constexpr auto kAddressZP = &GetZeroPageAddress;
constexpr auto kAddressZPX = &GetZeroPageIndirectAddressWithX;
constexpr auto kAddressZPY = &GetZeroPageIndirectAddressWithY;
constexpr auto kAddressABS = &GetAbsoluteAddress;

constexpr auto kAddressFastABSX = &GetAddressAbsoluteIndexedWithX<true>;
constexpr auto kAddressFastABSY = &GetAddressAbsoluteIndexedWithY<true>;
constexpr auto kAddressABSX = &GetAddressAbsoluteIndexedWithX<false>;
constexpr auto kAddressABSY = &GetAddressAbsoluteIndexedWithY<false>;

constexpr auto kAddressINDX = &GetAddresZeroPageIndexedIndirectWithX;
constexpr auto kAddressINDY = &GetAddresZeroPageIndirectIndexedWithY<true>;
constexpr auto kAddressSlowINDY = &GetAddresZeroPageIndirectIndexedWithY<false>;

} // namespace emu::emu6502::cpu::instructions
