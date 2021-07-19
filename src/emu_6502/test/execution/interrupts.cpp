#include "emu_core/byte_utils.hpp"
#include "execution_base_test.hpp"

namespace emu::emu6502::test {
namespace {

using namespace std::string_literals;

class InterruptsTest : public ExecutionTest, public ::testing::WithParamInterface<int> {};

TEST_F(InterruptsTest, brk) {
    auto code = R"==(
.isr reset TEST_ENTRY
.isr irq IRQ_HANDLER

.org 0x2000

TEST_ENTRY:
    LDX #$00
    JSR TEST_FUNC
    TXA
    HLT A

TEST_FUNC:
    BRK #$0
    BRK #$0

    BRK #$0
    BRK #$0

    BRK #$0
    BRK #$0

    BRK #$0
    BRK #$0

    RTS

IRQ_HANDLER:
    INX
    RTI

)=="s;

    auto program = RunCode(code);
    auto result = halt_code.value();

    std::cout << "R: " << static_cast<int>(result) << "\n";
    std::cout << "E: " << 8 << "\n";
    std::cout << "-----------DONE---------------------\n";
    EXPECT_EQ(result, 8);
}

} // namespace
} // namespace emu::emu6502::test
