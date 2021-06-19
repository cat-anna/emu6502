#include <algorithm>
#include <emu_6502/cpu/cpu.hpp>
#include <emu_6502/cpu/opcode.hpp>
#include <emu_core/base16.hpp>
#include <emu_core/clock.hpp>
#include <emu_core/memory.hpp>
#include <emu_core/memory_sparse.hpp>
#include <gtest/gtest.h>
#include <optional>

namespace emu::emu6502::test {
namespace {

class ResetTest : public ::testing::Test {
public:
    ClockSimple clock;
    MemorySparse16 memory{&clock, true, true};
    cpu::Cpu cpu{&clock, &memory, true, InstructionSet::NMOS6502Emu};

    ResetTest() {}
};

TEST_F(ResetTest, Reset) {
    memory.WriteRange(kResetVector, {0x55, 0xaa});
    cpu.Reset();
    EXPECT_EQ(cpu.reg.program_counter, 0xaa55);
}

} // namespace
} // namespace emu::emu6502::test