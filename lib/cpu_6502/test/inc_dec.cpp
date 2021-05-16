#include "base_test.hpp"
#include <gtest/gtest.h>
#include <optional>

namespace emu::cpu6502 {

using namespace emu::cpu6502::opcode;

class IncDecBaseTest : public BaseTest {
public:
    std::optional<uint8_t> result_byte;
    std::optional<MemPtr> operation_address;

    // Affects Flags: N Z

    void SetUp() override {
        BaseTest::SetUp();
        expected_regs.SetFlag(Flags::Zero, result_byte.value() == 0);
        expected_regs.SetFlag(Flags::Negative, (result_byte.value() & 0x80) > 0);
    }

    void Execute(const std::vector<uint8_t> &data, uint64_t cycles) override {
        // std::cout << fmt::format("TEST ADDR={:04x} DATA={:02x} ZP={:02x}\n", target_address.value(), target_byte,
        //                          zero_page_address);
        WriteMemory(operation_address.value(), {target_byte});
        BaseTest::Execute(data, cycles);
    }

    void TearDown() override {
        BaseTest::TearDown();
        VerifyMemory(operation_address.value(), {result_byte.value()});
    }
};

class INC : public IncDecBaseTest {
public:
    void SetUp() override {
        result_byte = static_cast<uint8_t>(target_byte + 1);
        IncDecBaseTest::SetUp();
    }
};

TEST_F(INC, INC_ZP) {
    // MODE           SYNTAX       HEX LEN TIM
    // Zero Page     INC $44       $E6  2   5
    operation_address = zero_page_address;
    Execute(MakeCode(INS_INC_ZP, zero_page_address), 5);
}

TEST_F(INC, INC_ZPX) {
    // MODE           SYNTAX       HEX LEN TIM
    // Zero Page,X   INC $44,X     $F6  2   6
    operation_address = zero_page_address + expected_regs.x;
    Execute(MakeCode(INS_INC_ZPX, zero_page_address), 6);
}

TEST_F(INC, INC_ABS) {
    // MODE           SYNTAX       HEX LEN TIM
    // Absolute      INC $4400     $EE  3   6
    operation_address = test_address;
    Execute(MakeCode(INS_INC_ABS, test_address), 6);
}

TEST_F(INC, INC_ABSX) {
    // MODE           SYNTAX       HEX LEN TIM
    // Absolute,X    INC $4400,X   $FE  3   7
    operation_address = test_address + expected_regs.x;
    Execute(MakeCode(INS_INC_ABSX, test_address), 7);
}

class DEC : public IncDecBaseTest {
public:
    void SetUp() override {
        result_byte = static_cast<uint8_t>(target_byte - 1);
        IncDecBaseTest::SetUp();
    }
};

TEST_F(DEC, DEC_ZP) {
    // MODE           SYNTAX       HEX LEN TIM
    // Zero Page     DEC $44       $C6  2   5
    operation_address = zero_page_address;
    Execute(MakeCode(INS_DEC_ZP, zero_page_address), 5);
}

TEST_F(DEC, DEC_ZPX) {
    // MODE           SYNTAX       HEX LEN TIM
    // Zero Page,X   DEC $44,X     $D6  2   6
    operation_address = zero_page_address + expected_regs.x;
    Execute(MakeCode(INS_DEC_ZPX, zero_page_address), 6);
}

TEST_F(DEC, DEC_ABS) {
    // MODE           SYNTAX       HEX LEN TIM
    // Absolute      DEC $4400     $CE  3   6
    operation_address = test_address;
    Execute(MakeCode(INS_DEC_ABS, test_address), 6);
}

TEST_F(DEC, DEC_ABSX) {
    // MODE           SYNTAX       HEX LEN TIM
    // Absolute,X    DEC $4400,X   $DE  3   7
    operation_address = test_address + expected_regs.x;
    Execute(MakeCode(INS_DEC_ABSX, test_address), 7);
}

} // namespace emu::cpu6502