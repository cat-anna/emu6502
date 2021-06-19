#include "base_test.hpp"
#include <functional>
#include <gtest/gtest.h>
#include <optional>

namespace emu::emu6502::test {
namespace {

using StackTestArg = std::tuple<Opcode, const char *, uint8_t, Reg8Ptr>;

class StackTest : public BaseTest, public ::testing::WithParamInterface<StackTestArg> {
public:
    // Stack Instructions
    // These instructions are implied mode, have a length of one byte and require machine cycles as indicated.
    // The "PuLl" operations are known as "POP" on most other microprocessors. With the 6502, the stack is always
    // on page one ($100-$1FF) and works top down.

    void SetUp() override { //
        BaseTest::SetUp();

        expected_regs.stack_pointer = 0x80;
        cpu.reg = expected_regs;
        expected_code_length = 1;
    }
};

TEST_F(StackTest, PHA) {
    // MNEMONIC                        HEX TIM
    // PHA (PusH Accumulator)          $48  3
    target_address = expected_regs.StackPointerMemoryAddress();
    memory.WriteRange(target_address, {0});
    expected_regs.stack_pointer--;
    expected_regs.a = cpu.reg.a = target_byte;
    expected_cycles = 3;
    Execute(MakeCode(INS_PHA));
    VerifyMemory(target_address, {target_byte});
}

TEST_F(StackTest, PLA) {
    // MNEMONIC                        HEX TIM
    // PLA (PuLl Accumulator)          $68  4
    expected_regs.stack_pointer++;
    target_address = expected_regs.StackPointerMemoryAddress();
    WriteMemory(target_address, {target_byte});
    expected_regs.SetFlag(Flags::Negative, (target_byte & 0x80) > 0);
    expected_regs.SetFlag(Flags::Zero, target_byte == 0);
    expected_regs.a = target_byte;
    expected_cycles = 4;
    Execute(MakeCode(INS_PLA));
}

TEST_F(StackTest, PHP) {
    // MNEMONIC                        HEX TIM
    // PHP (PusH Processor status)     $08  3
    target_address = expected_regs.StackPointerMemoryAddress();
    memory.WriteRange(target_address, {0});
    expected_regs.stack_pointer--;
    expected_regs.flags = cpu.reg.flags = target_byte;
    expected_cycles = 3;
    Execute(MakeCode(INS_PHP));
    uint8_t expected = target_byte | static_cast<uint8_t>(Flags::Brk) | static_cast<uint8_t>(Flags::NotUsed);
    VerifyMemory(target_address, {expected});
}

TEST_F(StackTest, PLP) {
    // MNEMONIC                        HEX TIM
    // PLP (PuLl Processor status)     $28  4
    target_address = expected_regs.StackPointerMemoryAddress();
    expected_regs.stack_pointer++;

    expected_regs.flags = target_byte & ~(static_cast<uint8_t>(Flags::Brk) | static_cast<uint8_t>(Flags::NotUsed));
    WriteMemory(target_address, {target_byte});

    expected_cycles = 4;
    Execute(MakeCode(INS_PLP));
}
} // namespace

} // namespace emu::emu6502::test