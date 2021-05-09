#include "base_test.hpp"
#include <gtest/gtest.h>
#include <optional>

using namespace emu6502::cpu::opcode;

class StoreBaseTest : public BaseTest {
public:
    std::optional<MemPtr> target_address;

    void Execute(const std::vector<uint8_t> &data, uint64_t cycles) override {
        std::cout << fmt::format("TEST ADDR={:04x} DATA={:02x} ZP={:02x}\n", target_address.value(), target_byte,
                                 zero_page_address);
        BaseTest::Execute(data, cycles);
    }

    void TearDown() override {
        BaseTest::TearDown();
        VerifyMemory(target_address.value(), {target_byte});
    }
};

class STA : public StoreBaseTest {
public:
    void SetUp() override {
        StoreBaseTest::SetUp();
        expected_regs.a = cpu.reg.a = target_byte;
    }
};

TEST_F(STA, STA_ABS) {
    // MODE           SYNTAX       HEX LEN TIM
    // Absolute      STA $4400     $8D  3   4
    target_address = test_address;
    Execute(MakeCode(INS_STA_ABS, test_address), 4);
}

TEST_F(STA, STA_ZP) {
    // MODE           SYNTAX       HEX LEN TIM
    // Zero Page     STA $44       $85  2   3
    target_address = zero_page_address;
    Execute(MakeCode(INS_STA_ZP, zero_page_address), 3);
}

TEST_F(STA, STA_ZPX) {
    // MODE           SYNTAX       HEX LEN TIM
    // Zero Page,X   STA $44,X     $95  2   4
    target_address = zero_page_address + expected_regs.x;
    Execute(MakeCode(INS_STA_ZPX, zero_page_address), 4);
}

TEST_F(STA, STA_ABSX) {
    // MODE           SYNTAX       HEX LEN TIM
    // Absolute,X    STA $4400,X   $9D  3   5
    target_address = test_address + expected_regs.x;
    Execute(MakeCode(INS_STA_ABSX, test_address), 5);
}

TEST_F(STA, STA_ABSY) {
    // MODE           SYNTAX       HEX LEN TIM
    // Absolute,Y    STA $4400,Y   $99  3   5
    target_address = test_address + expected_regs.y;
    Execute(MakeCode(INS_STA_ABSY, test_address), 5);
}

TEST_F(STA, STA_INDX) {
    // MODE           SYNTAX       HEX LEN TIM
    // Indirect,X    STA ($44,X)   $81  2   6
    auto indirect_target_address = zero_page_address + expected_regs.x;
    WriteMemory(indirect_target_address, {0xFF});
    target_address = 0x00FF;
    Execute(MakeCode(INS_STA_INDX, zero_page_address), 6);
}

TEST_F(STA, STA_INDY) {
    // MODE           SYNTAX       HEX LEN TIM
    // Indirect,Y    STA ($44),Y   $91  2   6
    WriteMemory(zero_page_address, {0xAA});
    target_address = 0xAA + expected_regs.y;
    Execute(MakeCode(INS_STA_INDY, zero_page_address), 6);
}

// TEST_F(STA, STA_INDZP) {
//     // MODE           SYNTAX       HEX LEN TIM
//     WriteMemory(zero_page_address, {0xAA});
//     target_address = 0xAA;
//     Execute(MakeCode(INS_STA_INDZP, zero_page_address), 0);
// }

class STX : public StoreBaseTest {
public:
    void SetUp() override {
        StoreBaseTest::SetUp();
        expected_regs.x = cpu.reg.x = target_byte;
    }
};

TEST_F(STX, STX_ZP) {
    // MODE           SYNTAX       HEX LEN TIM
    // Zero Page     STX $44       $86  2   3
    target_address = zero_page_address;
    Execute(MakeCode(INS_STX_ZP, zero_page_address), 3);
}

TEST_F(STX, STX_ZPY) {
    // MODE           SYNTAX       HEX LEN TIM
    // Zero Page,Y   STX $44,Y     $96  2   4
    target_address = zero_page_address + expected_regs.y;
    Execute(MakeCode(INS_STX_ZPY, zero_page_address), 4);
}

TEST_F(STX, STX_ABS) {
    // MODE           SYNTAX       HEX LEN TIM
    // Absolute      STX $4400     $8E  3   4
    target_address = test_address;
    Execute(MakeCode(INS_STX_ABS, test_address), 4);
}

class STY : public StoreBaseTest {
public:
    void SetUp() override {
        StoreBaseTest::SetUp();
        expected_regs.y = cpu.reg.y = target_byte;
    }
};

TEST_F(STY, STY_ZP) {
    // MODE           SYNTAX       HEX LEN TIM
    // Zero Page     STY $44       $84  2   3
    target_address = zero_page_address;
    Execute(MakeCode(INS_STY_ZP, zero_page_address), 3);
}

TEST_F(STY, STY_ZPX) {
    // MODE           SYNTAX       HEX LEN TIM
    // Zero Page,X   STY $44,X     $94  2   4
    target_address = zero_page_address + expected_regs.x;
    Execute(MakeCode(INS_STY_ZPX, zero_page_address), 4);
}

TEST_F(STY, STY_ABS) {
    // MODE           SYNTAX       HEX LEN TIM
    // Absolute      STY $4400     $8C  3   4
    target_address = test_address;
    Execute(MakeCode(INS_STY_ABS, test_address), 4);
}
