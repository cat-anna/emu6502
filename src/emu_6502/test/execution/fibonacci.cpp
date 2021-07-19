#include "emu_core/byte_utils.hpp"
#include "execution_base_test.hpp"

namespace emu::emu6502::test {
namespace {

using namespace std::string_literals;

uint64_t fibonacci(uint64_t v) {
    if (v == 0) {
        return 0;
    }
    if (v == 1) {
        return 1;
    }
    return fibonacci(v - 1) + fibonacci(v - 2);
}

class FibonacciTest : public ExecutionTest, public ::testing::WithParamInterface<int> {};

TEST_P(FibonacciTest, fibonacci_8) {
    auto code = R"==(
.isr reset TEST_ENTRY
.org 0x2000
TEST_ENTRY:
    LDX COUNT
    JSR FIBONACCI
    STX RESULT_VALUE
    TXA
    HLT A

FIBONACCI_0:
    LDX #$00
    RTS
FIBONACCI_1:
    LDX #$01
    RTS

; input  X
; output X
FIBONACCI:
    CPX #$00
    BEQ FIBONACCI_0
    CPX #$01
    BEQ FIBONACCI_1

    DEX            ; X=INPUT-1
    TXA            ; X=INPUT-1      A=INPUT-1
    PHA            ; X=INPUT-1      A=INPUT-1        [stack]=INPUT-1
    DEX            ; X=INPUT-2      A=INPUT-1        [stack]=INPUT-1
    JSR FIBONACCI  ; X=fib(INPUT-2) [stack]=INPUT-1

    PLA            ; X=fib(INPUT-2) A=INPUT-1
    TAY            ; X=fib(INPUT-2) A=INPUT-1        Y=INPUT-1
    TXA            ; X=fib(INPUT-2) A=fib(INPUT-2)   Y=INPUT-1
    PHA            ; X=fib(INPUT-2) A=fib(INPUT-2)   Y=INPUT-1        [stack]=fib(INPUT-2)
    TYA            ; X=fib(INPUT-2) A=INPUT-1        Y=INPUT-1        [stack]=fib(INPUT-2)
    TAX            ; X=INPUT-1      A=INPUT-1        Y=INPUT-1        [stack]=fib(INPUT-2)
    JSR FIBONACCI  ; X=fib(INPUT-1) [stack]=fib(INPUT-2)

    TXA            ; X=fib(INPUT-1) A=fib(INPUT-1)  [stack]=fib(INPUT-2)
    CLC            ; clear carry
    TSX            ; X=stack        A=fib(INPUT-1)  [stack]=fib(INPUT-2)
    INX            ; inc x,  to point to correct byte
    ADC $0100,X    ; A=fib(INPUT)   [stack]=fib(INPUT-2)
    TAX            ; A=fib(INPUT)   X=fib(INPUT)    [stack]=fib(INPUT-2)
    PLA            ; X=fib(INPUT)
    RTS

.org 0x3000
COUNT:
.byte {}

RESULT_VALUE:
.byte 0x00

)=="s;

    const uint8_t kTestValue = static_cast<uint8_t>(GetParam());

    auto program = RunCode(fmt::format(code, kTestValue));
    auto emulated_result = ByteVector{halt_code.value()};

    auto raw_Result = static_cast<uint8_t>(fibonacci(kTestValue));
    auto result = ToBytes(raw_Result);

    std::cout << "R: " << ToHexArray(emulated_result) << "\n";
    std::cout << "E: " << ToHexArray(result) << "\n";
    std::cout << "-----------DONE---------------------\n";
    EXPECT_EQ(emulated_result, result);
}

INSTANTIATE_TEST_SUITE_P(ExecutionTest, FibonacciTest, ::testing::Range(0, 14));

} // namespace
} // namespace emu::emu6502::test
