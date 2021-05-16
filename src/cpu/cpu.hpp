#pragma once

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <emu_core/clock.hpp>
#include <emu_core/memory.hpp>
#include <stdexcept>


namespace emu::cpu {

using Reg8 = uint8_t;
using Reg16 = uint16_t;

struct Cpu6502 {
    struct Registers {
        Reg16 program_counter;

        Reg8 a, x, y;
        Reg8 stack_pointer;

        Reg8 flags;

        enum class Flags {
            Carry = 0x01,
            Zero = 0x02,
            IRQB = 0x04,
            DecimalMode = 0x08,
            Brk = 0x10,
            NotUsed = 0x20,
            Overflow = 0x40,
            Negative = 0x80,
        };

        std::string DumpFlags() const {
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

        std::string Dump() const {
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

        bool TestFlag(Flags f) const { return (flags & static_cast<Reg8>(f)) == static_cast<Reg8>(f); }
        void SetFlag(Flags f, bool value) {
            if (value) {
                flags |= static_cast<Reg8>(f);
            } else {
                flags &= ~static_cast<Reg8>(f);
            }
        }

        void SetNegativeZeroFlag(uint8_t v) {
            SetFlag(Flags::Zero, v == 0);
            SetFlag(Flags::Negative, (v & 0x80) != 0);
        }
        void SetNegativeFlag(uint8_t v) { SetFlag(Flags::Negative, (v & 0x80) != 0); }

        uint8_t CarryValue() const { return TestFlag(Flags::Carry) ? 1 : 0; }

        MemPtr StackPointerMemoryAddress() const { return 0x0100 | stack_pointer; }
    };

    Registers reg;
    Clock *clock;
    Memory *memory;

    Cpu6502();

    void Execute();
    void ExecuteWithTimeout(std::chrono::microseconds timeout);
    void ExecuteUntil(uint64_t cycle);
    void ExecuteNextInstruction();

    using OperandFunctionPtr = void (Cpu6502::*)();
    using InstructionHandlerArray = std::array<OperandFunctionPtr, 256>;
    const InstructionHandlerArray instruction_handlers;

    static InstructionHandlerArray InitInstructionHandlerArray();

    void Reset() {
        reg.program_counter = 0;
        reg.a = reg.x = reg.y = 0;
        reg.stack_pointer = 0;
        reg.flags = 0;
    }

    using MemAddrFunc = MemPtr (Cpu6502::*)();
    using ByteReadFunc = uint8_t (Cpu6502::*)();
    using Reg8Ptr = Reg8(Registers::*);
    using LogicFunc = uint8_t (*)(uint8_t, uint8_t);
    using ShiftFunc = std::tuple<uint8_t, bool> (*)(uint8_t, bool);

private:
    MemPtr GetAbsoluteAddress() { // mode: a
        MemPtr addr = FetchNextByte();
        return addr | FetchNextByte() << 8;
    }
    MemPtr GetAddressAbsoluteIndexedIndirectWithX() { // mode (a,x)
        auto location = GetAbsoluteAddress() + reg.x;
        MemPtr addr = memory->Load(location++);
        return addr | memory->Load(location) << 8;
    }
    MemPtr GetAddressAbsoluteIndexedWithX() { // mode a,x
        return GetAbsoluteAddress() + reg.x;
    }
    MemPtr GetAddressAbsoluteIndexedWithY() { // mode a,y
        return GetAbsoluteAddress() + reg.y;
    }
    MemPtr GetAddressAbsoluteIndirect() { // mode (a)
        auto location = GetAbsoluteAddress();
        MemPtr addr = memory->Load(location++);
        return addr | memory->Load(location) << 8;
    }

    MemPtr GetAddressAccumulator() { // mode A
        return reg.a;
    }

    uint8_t FetchNextByte() { // mode #
        return memory->Load(reg.program_counter++);
    }

    MemPtr GetAddressProgramCounterRelative() { // mode r
        auto offset = FetchNextByte();
        return reg.program_counter + offset;
    }
    MemPtr GetStackAddress() { // mode s
        return reg.StackPointerMemoryAddress();
    }
    MemPtr GetZeroPageAddress() { //mode zp
        return FetchNextByte();
    }
    MemPtr GetAddresZeroPageIndexedIndirectWithX() { // mode (zp,x)
        return memory->Load(reg.x + GetZeroPageAddress());
    }
    MemPtr GetZeroPageIndirectAddressWithX() { // mode zp,x
        return GetZeroPageAddress() + reg.x;
    }
    MemPtr GetZeroPageIndirectAddressWithY() { // mode zp,y
        return GetZeroPageAddress() + reg.y;
    }
    MemPtr GetZeroPageIndirectAddress() { // mode (zp)
        return memory->Load(GetZeroPageAddress());
    }
    MemPtr GetAddresZeroPageIndirectIndexedWithY() { // mode (zp),y
        return memory->Load(GetZeroPageAddress()) + reg.y;
    }

    void NOP() { clock->WaitForNextCycle(); }

    template <MemAddrFunc addr_func>
    uint8_t FetchMemory() {
        return memory->Load((this->*addr_func)());
    }

    template <Reg8Ptr target, MemAddrFunc addr_func>
    void Reg8LD() {
        Reg8LD_raw<target, &Cpu6502::FetchMemory<addr_func>>();
    }

    template <Reg8Ptr target, ByteReadFunc read_func = &Cpu6502::FetchNextByte>
    void Reg8LD_raw() {
        auto value = (this->*read_func)();
        this->reg.*target = value;
        reg.SetNegativeZeroFlag(value);
    }

    template <Reg8Ptr target, MemAddrFunc addr_func>
    void Reg8ST() {
        auto value = this->reg.*target;
        memory->Store((this->*addr_func)(), value);
    }

    template <MemAddrFunc addr_func, int8_t direction>
    void Inc() {
        auto addr = (this->*addr_func)();
        auto value = memory->Load(addr) + direction;
        reg.SetNegativeZeroFlag(value);
        memory->Store(addr, value);
    }

    template <Reg8Ptr source, Reg8Ptr target, bool set_flags = true>
    void Reg8Transfer() {
        auto value = this->reg.*source;
        if constexpr (set_flags) {
            reg.SetNegativeZeroFlag(value);
        }
        this->reg.*target = value;
    }

    template <Reg8Ptr source, int8_t direction>
    void Reg8Inc() {
        auto value = this->reg.*source;
        value += direction;
        reg.SetNegativeZeroFlag(value);
        this->reg.*source = value;
    }

    template <ByteReadFunc read_func = &Cpu6502::FetchNextByte, bool subtract = false>
    void Reg8Add_template() {
        auto operand = (this->*read_func)();
        if (reg.TestFlag(Registers::Flags::DecimalMode)) {
            throw std::runtime_error("Decimal mode is not implemented yet");
        } else {
            const bool SignBitsAreEqual = ((reg.a ^ operand) & 0x80) == 0;

            uint16_t result = reg.a;
            result += reg.CarryValue();
            if (subtract) {
                result -= operand;
            } else {
                result += operand;
            }

            reg.a = result & 0xFF;
            reg.SetNegativeZeroFlag(result);
            reg.SetFlag(Registers::Flags::Carry, result > 0xFF);
            reg.SetFlag(Registers::Flags::Overflow, SignBitsAreEqual && ((reg.a ^ operand) & 0x80) != 0);
        }
    }

    template <ByteReadFunc read_func = &Cpu6502::FetchNextByte>
    void Reg8Adc_raw() {
        Reg8Add_template<read_func>();
    }

    template <ByteReadFunc read_func = &Cpu6502::FetchNextByte>
    void Reg8Sbc_raw() {
        Reg8Add_template<read_func, true>();
    }

    template <MemAddrFunc addr_func>
    void Reg8Adc() {
        Reg8Add_template<&Cpu6502::FetchMemory<addr_func>>();
    }

    template <MemAddrFunc addr_func>
    void Reg8Sbc() {
        Reg8Add_template<&Cpu6502::FetchMemory<addr_func>, true>();
    }

    template <Reg8Ptr source, ByteReadFunc read_func>
    void Reg8Compare() {
        auto src = this->reg.*source;
        auto operand = (this->*read_func)();
        reg.SetNegativeZeroFlag(src - operand);
        reg.SetFlag(Registers::Flags::Carry, src >= operand);
    }

    template <Registers::Flags flag, bool state>
    void Branch() {
        auto offset_address = static_cast<int8_t>(FetchNextByte());
        if (reg.TestFlag(flag) == state) {
            reg.program_counter += offset_address;
        }
    }

    template <LogicFunc op, ByteReadFunc read_func>
    void LogicalOp() {
        auto operand = (this->*read_func)();
        auto result = op(reg.a, operand);
        reg.SetNegativeZeroFlag(result);
        reg.a = result;
    }

    template <LogicFunc op, ByteReadFunc read_func>
    void BitOp() {
        auto operand = (this->*read_func)();
        auto result = op(reg.a, operand);
        reg.SetNegativeZeroFlag(result);
        reg.SetFlag(Registers::Flags::Overflow, (result & 0x40) > 0);
    }

    template <Registers::Flags flag, bool state>
    void SetFlag() {
        clock->WaitForNextCycle();
        reg.SetFlag(flag, state);
    }

    template <Reg8Ptr source, ShiftFunc op>
    void ShiftRegister() {
        auto operand = this->reg.*source;
        clock->WaitForNextCycle();
        auto [result, new_carry] = op(operand, reg.TestFlag(Registers::Flags::Carry));
        reg.SetNegativeZeroFlag(result);
        reg.SetFlag(Registers::Flags::Carry, new_carry);
        this->reg.*source = result;
    }

    template <ShiftFunc op, MemAddrFunc addr_func>
    void ShiftMemory() {
        auto addr = (this->*addr_func)();
        auto operand = memory->Load(addr);
        clock->WaitForNextCycle();
        auto [result, new_carry] = op(operand, reg.TestFlag(Registers::Flags::Carry));
        reg.SetNegativeZeroFlag(result);
        reg.SetFlag(Registers::Flags::Carry, new_carry);
        memory->Store(addr, result);
    }

    void StackPushByte(uint8_t v) {
        memory->Store(reg.StackPointerMemoryAddress(), v);
        clock->WaitForNextCycle();
        reg.stack_pointer--;
    }

    uint8_t StackPullByte() {
        auto operand = memory->Load(reg.StackPointerMemoryAddress());
        clock->WaitForNextCycle();
        reg.stack_pointer++;
        return operand;
    }

    template <Reg8Ptr source>
    void StackPush() {
        StackPushByte(this->reg.*source);
    }

    template <Reg8Ptr source>
    void StackPull() {
        auto operand = StackPullByte();
        clock->WaitForNextCycle();
        reg.SetNegativeZeroFlag(operand);
        this->reg.*source = operand;
    }

    void PushFlags() {
        auto operand = this->reg.flags | static_cast<uint8_t>(Registers::Flags::Brk) |
                       static_cast<uint8_t>(Registers::Flags::NotUsed);
        StackPushByte(operand);
    }

    void PullFlags() {
        auto operand = memory->Load(reg.StackPointerMemoryAddress());
        clock->WaitForNextCycle();
        this->reg.flags = operand;
        reg.SetFlag(Registers::Flags::Brk, false);
        reg.SetFlag(Registers::Flags::NotUsed, false);
        clock->WaitForNextCycle();
        reg.stack_pointer++;
    }

    void JumpABS() {
        auto addr = GetAbsoluteAddress();
        clock->WaitForNextCycle();
        reg.program_counter = addr;
    }

    void JumpIND() {
        auto addr = GetAbsoluteAddress();
        clock->WaitForNextCycle();
        MemPtr fetched_address = memory->Load(addr);
        addr = (addr & 0xFF00) | ((addr + 1) & 0xFF);
        fetched_address |= memory->Load(addr) << 8;
        reg.program_counter = fetched_address;
    }

    void JSR() { //
        auto addr = GetAbsoluteAddress();
        reg.program_counter -= 1;
        StackPushByte(reg.program_counter >> 8);
        StackPushByte(reg.program_counter & 0xff);
        reg.program_counter = addr;
    }

    void RTS() { //
        uint16_t low = StackPullByte();
        uint16_t hi = StackPullByte();
        clock->WaitForNextCycle();
        clock->WaitForNextCycle();
        reg.program_counter = (hi << 8 | low) + 1;
    }
    void RTI() {
        uint16_t low = StackPullByte();
        uint16_t hi = StackPullByte();
        clock->WaitForNextCycle();
        clock->WaitForNextCycle();
        reg.program_counter = (hi << 8 | low);
        reg.flags = StackPullByte();
    }
    void BRK() {
        clock->WaitForNextCycle();
        ++reg.program_counter;
        throw std::runtime_error("CPU halted");
    }
};

constexpr MemPtr kIrqVector = 0xFFFE;
constexpr MemPtr kResBVector = 0xFFFC;
constexpr MemPtr kNmibVector = 0xFFFA;

} // namespace emu::cpu