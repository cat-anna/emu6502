#include "base_test.hpp"
#include <gtest/gtest.h>
#include <optional>

namespace emu::cpu6502 {

using namespace emu::cpu6502::opcode;

class LoadBaseTest : public BaseTest {
public:
    std::optional<MemPtr> source_address;

    void SetUp() override {
        BaseTest::SetUp();
        std::cout << fmt::format("TEST ADDR={:04x} DATA={:02x} \n", test_address, target_byte);

        expected_regs.SetFlag(Flags::Zero, target_byte == 0);
        expected_regs.SetFlag(Flags::Negative, (target_byte & 0x80) > 0);
    }

    void Execute(const std::vector<uint8_t> &data, uint64_t cycles) override {
        if (source_address.has_value()) {
            WriteMemory(source_address.value(), {target_byte});
        }
        BaseTest::Execute(data, cycles);
    }
};

class LDA : public LoadBaseTest {
public:
    void SetUp() override {
        LoadBaseTest::SetUp();
        expected_regs.a = target_byte;
    }
};

TEST_F(LDA, LDA_ABS) {
    // MODE           SYNTAX       HEX LEN TIM
    // Absolute      LDA $4400     $AD  3   4
    source_address = test_address;
    Execute(MakeCode(INS_LDA_ABS, test_address), 4);
}

TEST_F(LDA, LDA_IM) {
    // MODE           SYNTAX       HEX LEN TIM
    // Immediate     LDA #$44      $A9  2   2
    Execute(MakeCode(INS_LDA_IM, target_byte), 2);
}

TEST_F(LDA, LDA_ZP) {
    // MODE           SYNTAX       HEX LEN TIM
    // Zero Page     LDA $44       $A5  2   3
    source_address = zero_page_address;
    Execute(MakeCode(INS_LDA_ZP, zero_page_address), 3);
}

TEST_F(LDA, LDA_ZPX) {
    // MODE           SYNTAX       HEX LEN TIM
    // Zero Page,X   LDA $44,X     $B5  2   4
    source_address = target_byte + expected_regs.x;
    Execute(MakeCode(INS_LDA_ZPX, target_byte), 4);
}

TEST_F(LDA, LDA_ABSX) {
    // MODE           SYNTAX       HEX LEN TIM
    // Absolute,X    LDA $4400,X   $BD  3   4+
    // + add 1 cycle if page boundary crossed
    source_address = test_address + expected_regs.x;
    Execute(MakeCode(INS_LDA_ABSX, test_address), 4);
}

TEST_F(LDA, LDA_ABSY) {
    // MODE           SYNTAX       HEX LEN TIM
    // Absolute,Y    LDA $4400,Y   $B9  3   4+
    // + add 1 cycle if page boundary crossed
    source_address = test_address + expected_regs.y;
    Execute(MakeCode(INS_LDA_ABSY, test_address), 4);
}

TEST_F(LDA, LDA_INDX) {
    // MODE           SYNTAX       HEX LEN TIM
    // Indirect,X    LDA ($44,X)   $A1  2   6
    WriteMemory(zero_page_address + expected_regs.x, {0xdd});
    source_address = 0xdd;
    Execute(MakeCode(INS_LDA_INDX, zero_page_address), 6);
}

TEST_F(LDA, LDA_INDY) {
    // MODE           SYNTAX       HEX LEN TIM
    // Indirect,Y    LDA ($44),Y   $B1  2   5+
    // + add 1 cycle if page boundary crossed
    WriteMemory(zero_page_address, {0x0d});
    source_address = 0x0d + expected_regs.y;
    Execute(MakeCode(INS_LDA_INDY, zero_page_address), 5);
}

// TEST_F(LDA, LDA_INDZP) {
//     // MODE           SYNTAX       HEX LEN TIM
//     WriteMemory(zero_page_address, {0x0d});
//     source_address = 0x0d;
//     Execute(MakeCode(INS_LDA_INDZP, zero_page_address), 0);
// }

class LDX : public LoadBaseTest {
public:
    void SetUp() override {
        LoadBaseTest::SetUp();
        expected_regs.x = target_byte;
    }
};

TEST_F(LDX, LDX_IM) {
    // MODE           SYNTAX       HEX LEN TIM
    // Immediate     LDX #$44      $A2  2   2
    Execute(MakeCode(INS_LDX_IM, target_byte), 2);
}

TEST_F(LDX, LDX_ZP) {
    // MODE           SYNTAX       HEX LEN TIM
    // Zero Page     LDX $44       $A6  2   3
    source_address = zero_page_address;
    Execute(MakeCode(INS_LDX_ZP, zero_page_address), 3);
}

TEST_F(LDX, LDX_ZPY) {
    // MODE           SYNTAX       HEX LEN TIM
    // Zero Page,Y   LDX $44,Y     $B6  2   4
    source_address = zero_page_address + expected_regs.y;
    Execute(MakeCode(INS_LDX_ZPY, zero_page_address), 4);
}

TEST_F(LDX, LDX_ABS) {
    // MODE           SYNTAX       HEX LEN TIM
    // Absolute      LDX $4400     $AE  3   4
    source_address = test_address;
    Execute(MakeCode(INS_LDX_ABS, test_address), 4);
}

TEST_F(LDX, LDX_ABSY) {
    // MODE           SYNTAX       HEX LEN TIM
    // Absolute,Y    LDX $4400,Y   $BE  3   4+
    // + add 1 cycle if page boundary crossed
    source_address = test_address + expected_regs.y;
    Execute(MakeCode(INS_LDX_ABSY, test_address), 4);
}

class LDY : public LoadBaseTest {
public:
    void SetUp() override {
        LoadBaseTest::SetUp();
        expected_regs.y = target_byte;
    }
};

TEST_F(LDY, LDY_IM) {
    // MODE           SYNTAX       HEX LEN TIM
    // Immediate     LDY #$44      $A0  2   2
    Execute(MakeCode(INS_LDY_IM, target_byte), 2);
}

TEST_F(LDY, LDY_ZP) {
    // MODE           SYNTAX       HEX LEN TIM
    // Zero Page     LDY $44       $A4  2   3
    source_address = zero_page_address;
    Execute(MakeCode(INS_LDY_ZP, zero_page_address), 3);
}

TEST_F(LDY, LDY_ZPX) {
    // MODE           SYNTAX       HEX LEN TIM
    // Zero Page,X   LDY $44,X     $B4  2   4
    source_address = zero_page_address + expected_regs.x;
    Execute(MakeCode(INS_LDY_ZPX, zero_page_address), 4);
}

TEST_F(LDY, LDY_ABS) {
    // MODE           SYNTAX       HEX LEN TIM
    // Absolute      LDY $4400     $AC  3   4
    source_address = test_address;
    Execute(MakeCode(INS_LDY_ABS, test_address), 4);
}

TEST_F(LDY, LDY_ABSX) {
    // MODE           SYNTAX       HEX LEN TIM
    // Absolute,X    LDY $4400,X   $BC  3   4+
    // + add 1 cycle if page boundary crossed
    source_address = test_address + expected_regs.x;
    Execute(MakeCode(INS_LDY_ABSX, test_address), 4);
}
} // namespace emu::cpu6502