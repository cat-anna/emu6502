#pragma once

#include <cstdint>
#include <string_view>
#include <unordered_map>

namespace emu6502::cpu::opcode {

using Opcode = uint8_t;

std::unordered_map<Opcode, std::string_view> Get6502InstructionSet();

// constexpr Opcode INS_STA_INDZP = 0x92;
// constexpr Opcode INS_LDA_INDZP = 0xB2;
// constexpr Opcode INS_INC_ACC = 0x1A;
// constexpr Opcode INS_DEC_ACC = 0x3A;

//LDA
constexpr Opcode INS_LDA_IM = 0xA9;
constexpr Opcode INS_LDA_ZP = 0xA5;
constexpr Opcode INS_LDA_ZPX = 0xB5;
constexpr Opcode INS_LDA_ABS = 0xAD;
constexpr Opcode INS_LDA_ABSX = 0xBD;
constexpr Opcode INS_LDA_ABSY = 0xB9;
constexpr Opcode INS_LDA_INDX = 0xA1;
constexpr Opcode INS_LDA_INDY = 0xB1;
//LDX
constexpr Opcode INS_LDX_IM = 0xA2;
constexpr Opcode INS_LDX_ZP = 0xA6;
constexpr Opcode INS_LDX_ZPY = 0xB6;
constexpr Opcode INS_LDX_ABS = 0xAE;
constexpr Opcode INS_LDX_ABSY = 0xBE;
//LDY
constexpr Opcode INS_LDY_IM = 0xA0;
constexpr Opcode INS_LDY_ZP = 0xA4;
constexpr Opcode INS_LDY_ZPX = 0xB4;
constexpr Opcode INS_LDY_ABS = 0xAC;
constexpr Opcode INS_LDY_ABSX = 0xBC;
//STA
constexpr Opcode INS_STA_ZP = 0x85;
constexpr Opcode INS_STA_ZPX = 0x95;
constexpr Opcode INS_STA_ABS = 0x8D;
constexpr Opcode INS_STA_ABSX = 0x9D;
constexpr Opcode INS_STA_ABSY = 0x99;
constexpr Opcode INS_STA_INDX = 0x81;
constexpr Opcode INS_STA_INDY = 0x91;
//STX
constexpr Opcode INS_STX_ZP = 0x86;
constexpr Opcode INS_STX_ZPY = 0x96;
constexpr Opcode INS_STX_ABS = 0x8E;
//STY
constexpr Opcode INS_STY_ZP = 0x84;
constexpr Opcode INS_STY_ZPX = 0x94;
constexpr Opcode INS_STY_ABS = 0x8C;

constexpr Opcode INS_TSX = 0xBA;
constexpr Opcode INS_TXS = 0x9A;
constexpr Opcode INS_PHA = 0x48;
constexpr Opcode INS_PLA = 0x68;
constexpr Opcode INS_PHP = 0x08;
constexpr Opcode INS_PLP = 0x28;

constexpr Opcode INS_JMP_ABS = 0x4C;
constexpr Opcode INS_JMP_IND = 0x6C;
constexpr Opcode INS_JSR = 0x20;
constexpr Opcode INS_RTS = 0x60;

//Logical Ops

//AND
constexpr Opcode INS_AND_IM = 0x29;
constexpr Opcode INS_AND_ZP = 0x25;
constexpr Opcode INS_AND_ZPX = 0x35;
constexpr Opcode INS_AND_ABS = 0x2D;
constexpr Opcode INS_AND_ABSX = 0x3D;
constexpr Opcode INS_AND_ABSY = 0x39;
constexpr Opcode INS_AND_INDX = 0x21;
constexpr Opcode INS_AND_INDY = 0x31;

//OR
constexpr Opcode INS_ORA_IM = 0x09;
constexpr Opcode INS_ORA_ZP = 0x05;
constexpr Opcode INS_ORA_ZPX = 0x15;
constexpr Opcode INS_ORA_ABS = 0x0D;
constexpr Opcode INS_ORA_ABSX = 0x1D;
constexpr Opcode INS_ORA_ABSY = 0x19;
constexpr Opcode INS_ORA_INDX = 0x01;
constexpr Opcode INS_ORA_INDY = 0x11;

//EOR
constexpr Opcode INS_EOR_IM = 0x49;
constexpr Opcode INS_EOR_ZP = 0x45;
constexpr Opcode INS_EOR_ZPX = 0x55;
constexpr Opcode INS_EOR_ABS = 0x4D;
constexpr Opcode INS_EOR_ABSX = 0x5D;
constexpr Opcode INS_EOR_ABSY = 0x59;
constexpr Opcode INS_EOR_INDX = 0x41;
constexpr Opcode INS_EOR_INDY = 0x51;

//BIT
constexpr Opcode INS_BIT_ZP = 0x24;
constexpr Opcode INS_BIT_ABS = 0x2C;

//Transfer Registers
constexpr Opcode INS_TAX = 0xAA;
constexpr Opcode INS_TAY = 0xA8;
constexpr Opcode INS_TXA = 0x8A;
constexpr Opcode INS_TYA = 0x98;

//Increments, Decrements
constexpr Opcode INS_INX = 0xE8;
constexpr Opcode INS_INY = 0xC8;
constexpr Opcode INS_DEX = 0xCA;
constexpr Opcode INS_DEY = 0x88;

constexpr Opcode INS_DEC_ZP = 0xC6;
constexpr Opcode INS_DEC_ZPX = 0xD6;
constexpr Opcode INS_DEC_ABS = 0xCE;
constexpr Opcode INS_DEC_ABSX = 0xDE;

constexpr Opcode INS_INC_ZP = 0xE6;
constexpr Opcode INS_INC_ZPX = 0xF6;
constexpr Opcode INS_INC_ABS = 0xEE;
constexpr Opcode INS_INC_ABSX = 0xFE;

//branches
constexpr Opcode INS_BEQ = 0xF0;
constexpr Opcode INS_BNE = 0xD0;
constexpr Opcode INS_BCS = 0xB0;
constexpr Opcode INS_BCC = 0x90;
constexpr Opcode INS_BMI = 0x30;
constexpr Opcode INS_BPL = 0x10;
constexpr Opcode INS_BVC = 0x50;
constexpr Opcode INS_BVS = 0x70;

//status flag changes
constexpr Opcode INS_CLC = 0x18;
constexpr Opcode INS_SEC = 0x38;
constexpr Opcode INS_CLD = 0xD8;
constexpr Opcode INS_SED = 0xF8;
constexpr Opcode INS_CLI = 0x58;
constexpr Opcode INS_SEI = 0x78;
constexpr Opcode INS_CLV = 0xB8;

//Arithmetic
constexpr Opcode INS_ADC = 0x69;
constexpr Opcode INS_ADC_ZP = 0x65;
constexpr Opcode INS_ADC_ZPX = 0x75;
constexpr Opcode INS_ADC_ABS = 0x6D;
constexpr Opcode INS_ADC_ABSX = 0x7D;
constexpr Opcode INS_ADC_ABSY = 0x79;
constexpr Opcode INS_ADC_INDX = 0x61;
constexpr Opcode INS_ADC_INDY = 0x71;

constexpr Opcode INS_SBC = 0xE9;
constexpr Opcode INS_SBC_ABS = 0xED;
constexpr Opcode INS_SBC_ZP = 0xE5;
constexpr Opcode INS_SBC_ZPX = 0xF5;
constexpr Opcode INS_SBC_ABSX = 0xFD;
constexpr Opcode INS_SBC_ABSY = 0xF9;
constexpr Opcode INS_SBC_INDX = 0xE1;
constexpr Opcode INS_SBC_INDY = 0xF1;

// Register Comparison
constexpr Opcode INS_CMP = 0xC9;
constexpr Opcode INS_CMP_ZP = 0xC5;
constexpr Opcode INS_CMP_ZPX = 0xD5;
constexpr Opcode INS_CMP_ABS = 0xCD;
constexpr Opcode INS_CMP_ABSX = 0xDD;
constexpr Opcode INS_CMP_ABSY = 0xD9;
constexpr Opcode INS_CMP_INDX = 0xC1;
constexpr Opcode INS_CMP_INDY = 0xD1;

constexpr Opcode INS_CPX = 0xE0;
constexpr Opcode INS_CPY = 0xC0;
constexpr Opcode INS_CPX_ZP = 0xE4;
constexpr Opcode INS_CPY_ZP = 0xC4;
constexpr Opcode INS_CPX_ABS = 0xEC;
constexpr Opcode INS_CPY_ABS = 0xCC;

// shifts
constexpr Opcode INS_ASL = 0x0A;
constexpr Opcode INS_ASL_ZP = 0x06;
constexpr Opcode INS_ASL_ZPX = 0x16;
constexpr Opcode INS_ASL_ABS = 0x0E;
constexpr Opcode INS_ASL_ABSX = 0x1E;

constexpr Opcode INS_LSR = 0x4A;
constexpr Opcode INS_LSR_ZP = 0x46;
constexpr Opcode INS_LSR_ZPX = 0x56;
constexpr Opcode INS_LSR_ABS = 0x4E;
constexpr Opcode INS_LSR_ABSX = 0x5E;

constexpr Opcode INS_ROL = 0x2A;
constexpr Opcode INS_ROL_ZP = 0x26;
constexpr Opcode INS_ROL_ZPX = 0x36;
constexpr Opcode INS_ROL_ABS = 0x2E;
constexpr Opcode INS_ROL_ABSX = 0x3E;

constexpr Opcode INS_ROR = 0x6A;
constexpr Opcode INS_ROR_ZP = 0x66;
constexpr Opcode INS_ROR_ZPX = 0x76;
constexpr Opcode INS_ROR_ABS = 0x6E;
constexpr Opcode INS_ROR_ABSX = 0x7E;

//misc
constexpr Opcode INS_NOP = 0xEA;
constexpr Opcode INS_BRK = 0x00;
constexpr Opcode INS_RTI = 0x4;

} // namespace emu6502::cpu::opcode
