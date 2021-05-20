#include <algorithm>
#include <emu6502/cpu/cpu.hpp>
#include <emu6502/cpu/opcode.hpp>
#include <emu_core/base16.hpp>
#include <emu_core/clock.hpp>
#include <emu_core/memory.hpp>
#include <gtest/gtest.h>
#include <optional>

namespace emu::emu6502::test {
namespace {

class ResetTest : public ::testing::Test {
public:
    emu::Memory memory;
    cpu::Cpu cpu{InstructionSet::NMOS6502Emu};
    emu::Clock clock;

    ResetTest() {
        cpu.memory = &memory;
        cpu.clock = &clock;
        memory.clock = &clock;
    }
};

TEST_F(ResetTest, Reset) {
    memory.Write(kResetVector, {0x55, 0xaa});
    cpu.Reset();
    EXPECT_EQ(cpu.reg.program_counter, 0xaa55);
}

} // namespace
} // namespace emu::emu6502::test