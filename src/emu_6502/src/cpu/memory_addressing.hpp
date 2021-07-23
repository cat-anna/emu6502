#pragma once

#include "emu_6502/cpu/cpu.hpp"
#include "emu_core/memory.hpp"
#include <emu_core/clock.hpp>

namespace emu::emu6502::cpu::instructions {

template <bool wrap_address = true, bool add_cycle = false>
MemPtr AdvanceAddress(Cpu *cpu, MemPtr base, uint8_t v) {
    if constexpr (wrap_address) {
        return (base & 0xFF00) | ((base + v) & 0x00FF);
    } else {
        if constexpr (add_cycle) {
            if ((((base & 0xFF) + v) & 0xFF00) != 0) {
                cpu->WaitForNextCycle();
            }
        }
        return base + v;
    }
}

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
        cpu->WaitForNextCycle();
    }
    return AdvanceAddress<false, fast>(cpu, r, cpu->reg.x);
}

template <bool fast>
MemPtr GetAddressAbsoluteIndexedWithY(Cpu *cpu) { // mode a,y
    auto r = GetAbsoluteAddress(cpu);
    if constexpr (!fast) {
        cpu->WaitForNextCycle();
    }
    return AdvanceAddress<false, fast>(cpu, r, cpu->reg.y);
}

MemPtr GetAddressAbsoluteIndirect(Cpu *cpu) { // mode (a)
    auto location = GetAbsoluteAddress(cpu);
    MemPtr addr = cpu->memory->Load(location++);
    return addr | cpu->memory->Load(location) << 8;
}

uint8_t FetchAccumulator(Cpu *cpu) { // mode A
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

template <bool wrap_address = true>
MemPtr GetAddresZeroPageIndexedIndirectWithX(Cpu *cpu) { // mode (zp,x)
    //addr = PEEK((arg + X) % 256) +
    //       PEEK((arg + X + 1) % 256) * 256
    MemPtr arg = FetchNextByte(cpu);
    auto ind0 = AdvanceAddress<wrap_address, false>(cpu, arg, cpu->reg.x);
    auto low = cpu->memory->Load(ind0);

    cpu->WaitForNextCycle();
    auto ind1 = AdvanceAddress<wrap_address, false>(cpu, ind0, 1);
    auto hi = cpu->memory->Load(ind1);
    MemPtr addr = (hi << 8) | low;
    return addr;
}

template <bool wrap_address = true>
MemPtr GetZeroPageIndirectAddressWithX(Cpu *cpu) { // mode zp,x
    MemPtr zp = FetchNextByte(cpu);
    cpu->WaitForNextCycle();
    return AdvanceAddress<wrap_address>(cpu, zp, cpu->reg.x);
}

template <bool wrap_address = true>
MemPtr GetZeroPageIndirectAddressWithY(Cpu *cpu) { // mode zp,y
    MemPtr zp = FetchNextByte(cpu);
    cpu->WaitForNextCycle();
    return AdvanceAddress<wrap_address>(cpu, zp, cpu->reg.y);
}

MemPtr GetZeroPageIndirectAddress(Cpu *cpu) { // mode (zp)
    return cpu->memory->Load(GetZeroPageAddress(cpu));
}

template <bool always_add_cycle = false>
MemPtr GetAddresZeroPageIndirectIndexedWithY(Cpu *cpu) { // mode (zp),y
    // addr = PEEK(arg) +
    //        PEEK((arg + 1) % 256) * 256 +
    //        Y
    auto arg = FetchNextByte(cpu);
    auto low = cpu->memory->Load(arg);
    auto hi = cpu->memory->Load(AdvanceAddress<true, false>(cpu, arg, 1));
    if constexpr (always_add_cycle) {
        cpu->WaitForNextCycle();
    }
    MemPtr ind = (hi << 8) | low;
    return AdvanceAddress<false, !always_add_cycle>(cpu, ind, cpu->reg.y);
}

constexpr auto kAddressZP = &GetZeroPageAddress;
constexpr auto kAddressZPX = &GetZeroPageIndirectAddressWithX<true>;
constexpr auto kAddressZPY = &GetZeroPageIndirectAddressWithY<true>;
constexpr auto kAddressABS = &GetAbsoluteAddress;

constexpr auto kAddressFastABSX = &GetAddressAbsoluteIndexedWithX<true>;
constexpr auto kAddressFastABSY = &GetAddressAbsoluteIndexedWithY<true>;
constexpr auto kAddressABSX = &GetAddressAbsoluteIndexedWithX<false>;
constexpr auto kAddressABSY = &GetAddressAbsoluteIndexedWithY<false>;

constexpr auto kAddressINDX = &GetAddresZeroPageIndexedIndirectWithX<>;
constexpr auto kAddressINDY = &GetAddresZeroPageIndirectIndexedWithY<false>;
constexpr auto kAddressStoreINDY = &GetAddresZeroPageIndirectIndexedWithY<true>;

} // namespace emu::emu6502::cpu::instructions
