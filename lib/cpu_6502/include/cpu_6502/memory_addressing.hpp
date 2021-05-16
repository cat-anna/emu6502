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

template <bool fast>
MemPtr GetAddressAbsoluteIndexedWithX(Cpu6502 *cpu) { // mode a,x
    auto r = GetAbsoluteAddress(cpu);
    if constexpr (!fast) {
        cpu->clock->WaitForNextCycle(); //TODO: if across page ?
    }
    return r + cpu->reg.x;
}

template <bool fast>
MemPtr GetAddressAbsoluteIndexedWithY(Cpu6502 *cpu) { // mode a,y
    auto r = GetAbsoluteAddress(cpu);
    if constexpr (!fast) {
        cpu->clock->WaitForNextCycle(); //TODO: if across page ?
    }
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
    cpu->clock->WaitForNextCycle();
    return cpu->reg.program_counter + offset;
}

MemPtr GetStackAddress(Cpu6502 *cpu) { // mode s
    return cpu->reg.StackPointerMemoryAddress();
}

MemPtr GetZeroPageAddress(Cpu6502 *cpu) { //mode zp
    return FetchNextByte(cpu);
}

MemPtr GetAddresZeroPageIndexedIndirectWithX(Cpu6502 *cpu) { // mode (zp,x)
    MemPtr zp = GetZeroPageAddress(cpu);
    cpu->clock->WaitForNextCycle();
    MemPtr indirect = zp + cpu->reg.x;
    cpu->clock->WaitForNextCycle();
    return cpu->memory->Load(indirect);
}

MemPtr GetZeroPageIndirectAddressWithX(Cpu6502 *cpu) { // mode zp,x
    MemPtr zp = GetZeroPageAddress(cpu);
    cpu->clock->WaitForNextCycle();
    return zp + cpu->reg.x;
}

MemPtr GetZeroPageIndirectAddressWithY(Cpu6502 *cpu) { // mode zp,y
    MemPtr zp = GetZeroPageAddress(cpu);
    cpu->clock->WaitForNextCycle();
    return zp + cpu->reg.y;
}

MemPtr GetZeroPageIndirectAddress(Cpu6502 *cpu) { // mode (zp)
    return cpu->memory->Load(GetZeroPageAddress(cpu));
}

template <bool fast>
MemPtr GetAddresZeroPageIndirectIndexedWithY(Cpu6502 *cpu) { // mode (zp),y
    MemPtr zp = GetZeroPageAddress(cpu);
    cpu->clock->WaitForNextCycle();
    auto mem = cpu->memory->Load(zp);
    if constexpr (!fast) {
        cpu->clock->WaitForNextCycle();
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

} // namespace emu::cpu6502::instructions
