#pragma once

#include <cpu/clock.hpp>
#include <cpu/cpu.hpp>
#include <cpu/memory.hpp>
#include <cpu/opcode.hpp>
#include <gtest/gtest.h>
#include <string>
#include <vector>

enum class AddressMode : uint8_t {
    IM,
    ABS,
    ZP,
    ZPX,
    ABSX,
    ABSY,
    INDX,
    INDY,
    ACC,
};

inline std::string to_string(AddressMode mode) {
    switch (mode) {
    case AddressMode::IM:
        return "IM";
    case AddressMode::ABS:
        return "ABS";
    case AddressMode::ZP:
        return "ZP";
    case AddressMode::ZPX:
        return "ZPX";
    case AddressMode::ABSX:
        return "ABSX";
    case AddressMode::ABSY:
        return "ABSY";
    case AddressMode::INDY:
        return "INDY";
    case AddressMode::INDX:
        return "INDX";
    case AddressMode::ACC:
        return "ACC";
    }
    throw std::runtime_error("Invalid address mode");
}

class BaseTest : public testing::Test {
public:
    using MemPtr = emu6502::cpu::MemPtr;
    using Registers = emu6502::cpu::Cpu6502::Registers;
    using Flags = Registers::Flags;

    emu6502::cpu::Memory memory;
    emu6502::cpu::Cpu6502 cpu;
    emu6502::cpu::Clock clock;
    Registers expected_regs;

    static constexpr MemPtr kBaseCodeAddress = 0x1770;
    static constexpr MemPtr kBaseDataAddress = 0xE000;

    BaseTest() {
        cpu.memory = &memory;
        cpu.clock = &clock;
        memory.clock = &clock;

        cpu.Reset();
        expected_regs.a = 0x10;
        expected_regs.x = 0x20;
        expected_regs.y = 0x30;
        expected_regs.stack_pointer = 0x40;
        expected_regs.program_counter = kBaseCodeAddress;
        expected_regs.flags = RandomByte();
        cpu.reg = expected_regs;

        do {
            zero_page_address = RandomByte();
            indirect_address = RandomByte();
            target_byte = RandomByte();
            test_address = kBaseDataAddress | (RandomByte() & 0xF0);
            target_address = test_address;
        } while (indirect_address + expected_regs.y != zero_page_address &&
                 zero_page_address + expected_regs.x != indirect_address);
    }

    uint8_t zero_page_address{0};
    uint8_t indirect_address{0};
    uint8_t target_byte{0};
    MemPtr test_address{0};
    MemPtr target_address{0};

    bool is_testing_jumps = false;
    bool enable_cycles_verification = false;

    void SetUp() override { //
    }

    virtual void Execute(const std::vector<uint8_t> &data, uint64_t cycles) {
        if (!is_testing_jumps) {
            expected_regs.program_counter = kBaseCodeAddress + data.size();
        }
        std::cout << fmt::format(
            "SETUP target_byte=0x{:02x}; target_address={:04x} zero_page_address=0x{:02x}; indirect_address=0x{:02x}; "
            "test_address=0x{:04x};\n",
            target_byte, target_address, zero_page_address, indirect_address, test_address);
        std::cout << "CPU STATE 0: " << cpu.reg.Dump() << "\n";
        WriteMemory(kBaseCodeAddress, data);
        cpu.ExecuteNextInstruction();
        std::cout << "CPU STATE 1: " << cpu.reg.Dump() << "\n";
        std::cout << "CYCLES:" << clock.CurrentCycle() << "\n";

        if (enable_cycles_verification) {
            EXPECT_EQ(clock.CurrentCycle(), cycles); //TODO
        }
    }

    void TearDown() override {
        EXPECT_EQ(cpu.reg.flags, expected_regs.flags)
            << fmt::format("Expected:{} Actual:{}", expected_regs.DumpFlags(), cpu.reg.DumpFlags());

        EXPECT_EQ(expected_regs.stack_pointer, cpu.reg.stack_pointer)
            << fmt::format("Expected:{:02x} Actual:{:02x}", expected_regs.stack_pointer, cpu.reg.stack_pointer);
        EXPECT_EQ(expected_regs.x, cpu.reg.x)
            << fmt::format("Expected:{:02x} Actual:{:02x}", expected_regs.x, cpu.reg.x);
        EXPECT_EQ(expected_regs.y, cpu.reg.y)
            << fmt::format("Expected:{:02x} Actual:{:02x}", expected_regs.y, cpu.reg.y);
        EXPECT_EQ(expected_regs.a, cpu.reg.a)
            << fmt::format("Expected:{:02x} Actual:{:02x}", expected_regs.a, cpu.reg.a);

        EXPECT_EQ(expected_regs.program_counter, cpu.reg.program_counter)
            << fmt::format("Expected:{:02x} Actual:{:02x}", expected_regs.program_counter, cpu.reg.program_counter);
    }

    std::vector<uint8_t> MakeCode(uint8_t opcode, uint16_t arg) {
        return {opcode, static_cast<uint8_t>(arg & 0xFF), static_cast<uint8_t>(arg >> 8)};
    }
    std::vector<uint8_t> MakeCode(uint8_t opcode) { return {opcode}; }
    std::vector<uint8_t> MakeCode(uint8_t opcode, uint8_t arg) { return {opcode, arg}; }

    std::vector<uint8_t> MakeCode(uint8_t opcode, AddressMode mode) {
        WriteTestData(mode);

        switch (mode) {
        case AddressMode::IM:
            return MakeCode(opcode, target_byte);
        case AddressMode::ABS:
            return MakeCode(opcode, test_address);
        case AddressMode::ZP:
            return MakeCode(opcode, zero_page_address);
        case AddressMode::ZPX:
            return MakeCode(opcode, zero_page_address);
        case AddressMode::ABSX:
            return MakeCode(opcode, test_address);
        case AddressMode::ABSY:
            return MakeCode(opcode, test_address);
        case AddressMode::INDY:
            return MakeCode(opcode, zero_page_address);
        case AddressMode::INDX:
            return MakeCode(opcode, zero_page_address);
        case AddressMode::ACC:
            return MakeCode(opcode);
        }
        throw std::runtime_error("Invalid address mode");
    }

    void WriteMemory(MemPtr addr, const std::vector<uint8_t> &data) { memory.Write(addr, data); }
    void VerifyMemory(MemPtr addr, const std::vector<uint8_t> &data) {
        auto content = memory.ReadRange(addr, data.size());
        EXPECT_EQ(data, content) << fmt::format("Base address: {:04x}", addr);
    }

    uint8_t RandomByte() { return rand() & 0xFF; }

    void WriteTestData(AddressMode mode) {
        switch (mode) {
        case AddressMode::IM:
        case AddressMode::ACC:
            return;
        case AddressMode::ABS:
            target_address = test_address;
            break;
        case AddressMode::ZP:
            target_address = zero_page_address;
            break;
        case AddressMode::ZPX:
            target_address = zero_page_address + expected_regs.x;
            break;
        case AddressMode::ABSX:
            target_address = test_address + expected_regs.x;
            break;
        case AddressMode::ABSY:
            target_address = test_address + expected_regs.y;
            break;
        case AddressMode::INDX:
            WriteMemory(static_cast<MemPtr>(zero_page_address) + expected_regs.x, {indirect_address});
            target_address = indirect_address;
            break;
        case AddressMode::INDY:
            WriteMemory(zero_page_address, {indirect_address});
            target_address = static_cast<MemPtr>(indirect_address) + expected_regs.y;
            break;
        }

        WriteMemory(target_address, {target_byte});
    }
};

inline auto GenTestNameFunc(std::string caption = "") {
    if (!caption.empty()) {
        caption += "_";
    }
    return [caption = std::move(caption)](const auto &info) -> std::string {
        return fmt::format("{}{}", caption, to_string(std::get<1>(info.param)));
    };
}