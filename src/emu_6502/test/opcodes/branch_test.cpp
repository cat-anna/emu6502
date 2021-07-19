#include "base_test.hpp"
#include <gtest/gtest.h>
#include <optional>

namespace emu::emu6502::test {
namespace {

using BranchTestArg = std::tuple<Opcode, std::string, Registers::Flags, bool>;

class BranchTest : public BaseTest, public ::testing::WithParamInterface<BranchTestArg> {
public:
    // Branch Instructions
    // Affect Flags: none

    // All branches are relative mode and have a length of two bytes.
    // Syntax is "Bxx Displacement" or (better) "Bxx Label". See
    // the notes on the Program Counter for more on displacements.

    // Branches are dependant on the status of the flag bits when
    // the op code is encountered. A branch not taken requires two
    // machine cycles. Add one if the branch is taken and add one more
    // if the branch crosses a page boundary.

    // There is no BRA (BRanch Always) instruction
    // but it can be easily emulated by branching
    // on the basis of a known condition. One of the
    // best flags to use for this purpose is the
    // oVerflow which is unchanged by all but
    // addition and subtraction operations.

    // A page boundary crossing occurs when the branch
    // destination is on a different page than the instruction
    // AFTER the branch instruction. For example:

    //   SEC
    //   BCS LABEL
    //   NOP
    // A page boundary crossing occurs (i.e. the BCS takes 4 cycles)
    // when (the address of) LABEL and the NOP are on different pages. This means that
    //         CLV
    //         BVC LABEL
    //   LABEL NOP
    // the BVC instruction will take 3 cycles no matter what address it is located at.

    static constexpr uint8_t kBranchIntructionLength = 2;
    static constexpr uint8_t kBranchIntructionDuration = 2;

    static constexpr int8_t kNearJump = 0x10;
    static constexpr int8_t kFarJump = static_cast<int8_t>(0x7F);
    static constexpr int8_t kBackwardJump = static_cast<int8_t>(0xC5);

    void ExecuteBranchTest(int8_t jump_offset, uint64_t cycles, bool branch_successful) {
        is_testing_jumps = branch_successful;
        expected_cycles = kBranchIntructionDuration + cycles;
        expected_code_length = kBranchIntructionLength;

        auto [opcode, name, flag, state] = GetParam();
        if (!branch_successful) {
            state = !state;
        }

        expected_regs.SetFlag(flag, state);

        if (branch_successful) {
            expected_regs.program_counter += jump_offset;
            expected_regs.program_counter += kBranchIntructionLength;
        }

        cpu.reg.flags = expected_regs.flags;

        BaseTest::Execute(MakeCode(opcode, (uint8_t)jump_offset));
    }
};

TEST_P(BranchTest, BranchFails) {
    auto [opcode, name, flag, state] = GetParam();
    ExecuteBranchTest(kNearJump, 0, false);
}

TEST_P(BranchTest, SamePage) {
    auto [opcode, name, flag, state] = GetParam();
    ExecuteBranchTest(kNearJump, 1, true);
}

TEST_P(BranchTest, FarJump) {
    cpu.reg.program_counter = expected_regs.program_counter = 0x17F0;
    auto [opcode, name, flag, state] = GetParam();
    ExecuteBranchTest(kFarJump, 2, true);
}

TEST_P(BranchTest, Backward) {
    auto [opcode, name, flag, state] = GetParam();
    ExecuteBranchTest(kBackwardJump, 1, true);
}

std::vector<BranchTestArg> GetBranchTestCases() {
    return {
        {INS_BCC, "BCC", Flags::Carry, false},    //
        {INS_BCS, "BCS", Flags::Carry, true},     //
        {INS_BEQ, "BEQ", Flags::Zero, true},      //
        {INS_BNE, "BNE", Flags::Zero, false},     //
        {INS_BMI, "BMI", Flags::Negative, true},  //
        {INS_BPL, "BPL", Flags::Negative, false}, //
        {INS_BVC, "BVC", Flags::Overflow, false}, //
        {INS_BVS, "BVS", Flags::Overflow, true},  //
    };
}

INSTANTIATE_TEST_SUITE_P(, BranchTest, ::testing::ValuesIn(GetBranchTestCases()),
                         GenTestNameFunc());

class JumpTest : public BaseTest {
public:
    void SetUp() override { //
        BaseTest::SetUp();
        is_testing_jumps = true;
        expected_code_length = 3;
    }
};

// JMP (JuMP)
// Affects Flags: none

// JMP transfers program execution to the following address (absolute) or to the location contained
// in the following address (indirect). Note that there is no carry associated with the indirect jump so:
// AN INDIRECT JUMP MUST NEVER USE A
// VECTOR BEGINNING ON THE LAST BYTE
// OF A PAGE
// For example if address $3000 contains $40, $30FF contains $80, and $3100 contains $50, the result
// of JMP ($30FF) will be a transfer of control to $4080 rather than $5080 as you intended i.e. the
// 6502 took the low byte of the address from $30FF and the high byte from $3000.

TEST_F(JumpTest, JMP_ABS) {
    // MODE           SYNTAX       HEX LEN TIM
    // Absolute      JMP $5597     $4C  3   3
    expected_regs.program_counter = test_address;
    expected_cycles = 3;
    expected_code_length = 3;
    Execute(MakeCode(INS_JMP_ABS, test_address));
}

TEST_F(JumpTest, JMP_IND) {
    // MODE           SYNTAX       HEX LEN TIM
    // Indirect      JMP ($5597)   $6C  3   5
    WriteMemory(test_address, {0x10, 0x20});
    expected_regs.program_counter = 0x2010;
    expected_cycles = 5;
    expected_code_length = 3;
    Execute(MakeCode(INS_JMP_IND, test_address));
}

TEST_F(JumpTest, JMP_IND_AcrossPage) {
    // MODE           SYNTAX       HEX LEN TIM
    // Indirect      JMP ($5597)   $6C  3   5
    test_address = 0x30FF;
    WriteMemory(0x3000, {0x40});
    WriteMemory(0x30FF, {0x80});
    WriteMemory(0x3100, {0x50});
    expected_regs.program_counter = 0x4080;
    expected_cycles = 5;
    expected_code_length = 3;
    Execute(MakeCode(INS_JMP_IND, test_address));
}

// JSR (Jump to SubRoutine)
// Affects Flags: none

// JSR pushes the address-1 of the next operation on to the stack before transferring program control
// to the following address. Subroutines are normally terminated by a RTS op code.

TEST_F(JumpTest, JSR) {
    // MODE           SYNTAX       HEX LEN TIM
    // Absolute      JSR $5597     $20  3   6
    expected_regs.program_counter = test_address;
    expected_regs.stack_pointer -= 2;
    expected_cycles = 6;
    expected_code_length = 3;
    memory.WriteRange(expected_regs.StackPointerMemoryAddress() + 1, {0, 0});
    Execute(MakeCode(INS_JSR, test_address));
    auto pc = kBaseCodeAddress + 2;
    VerifyMemory(expected_regs.StackPointerMemoryAddress() + 1,
                 {(uint8_t)(pc & 0xFF), (uint8_t)(pc >> 8)});
}

// RTS (ReTurn from Subroutine)
// Affects Flags: none

// RTS pulls the top two bytes off the stack (low byte first) and transfers program
// control to that address+1. It is used, as expected, to exit a subroutine invoked
// via JSR which pushed the address-1.
// RTS is frequently used to implement a jump table where addresses-1 are pushed onto
// the stack and accessed via RTS eg. to access the second of four routines:

TEST_F(JumpTest, RTS) {
    // MODE           SYNTAX       HEX LEN TIM
    // Implied       RTS           $60  1   6
    auto pc = test_address - 1;
    expected_regs.stack_pointer += 1;
    WriteMemory(expected_regs.StackPointerMemoryAddress(),
                {(uint8_t)(pc & 0xFF), (uint8_t)(pc >> 8)});
    expected_regs.stack_pointer += 1;
    expected_regs.program_counter = test_address;
    expected_cycles = 6;
    expected_code_length = 1;
    Execute(MakeCode(INS_RTS));
}

// RTI (ReTurn from Interrupt)
// Affects Flags: all

// MODE           SYNTAX       HEX LEN TIM
// Implied       RTI           $40  1   6

// RTI retrieves the Processor Status Word (flags) and the Program Counter
// from the stack in that order (interrupts push the PC first and then the PSW).
// Note that unlike RTS, the return address on the stack is the actual address
// rather than the address-1.

TEST_F(JumpTest, RTI) {
    // MODE           SYNTAX       HEX LEN TIM
    // Implied       RTS           $60  1   6

    auto pc = test_address;
    cpu.reg.flags = RandomByte();
    expected_regs.stack_pointer += 1;
    WriteMemory(expected_regs.StackPointerMemoryAddress(),
                {expected_regs.flags, (uint8_t)(pc & 0xFF), (uint8_t)(pc >> 8)});
    expected_regs.stack_pointer += 2;
    expected_regs.program_counter = test_address;
    expected_regs.SetFlag(Flags::Brk, false);
    expected_regs.SetFlag(Flags::NotUsed, false);
    expected_cycles = 6;
    expected_code_length = 1;
    is_testing_jumps = true;
    Execute(MakeCode(INS_RTI));
}

TEST_F(JumpTest, BRK) {
    // BRK (BReaK)
    // Affects Flags: B

    // MODE           SYNTAX       HEX LEN TIM
    // Implied       BRK           $00  1   7

    // BRK causes a non-maskable interrupt and increments the program counter by one.
    // Therefore an RTI will go to the address of the BRK +2 so that BRK may be used
    // to replace a two-byte instruction for debugging and the subsequent RTI will be correct.

    expected_code_length = 2;
    expected_cycles = 7;
    is_testing_jumps = true;

    WriteMemory(kIrqVector, ToBytes(test_address));

    expected_regs.program_counter = test_address;
    expected_regs.SetFlag(Flags::IRQB, true);
    ExpectStackWrite(3);

    Execute(MakeCode(INS_BRK, (uint8_t)1));
}

} // namespace
} // namespace emu::emu6502::test