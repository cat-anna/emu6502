#include "opcode.hpp"

namespace emu6502::cpu::opcode {

using namespace std::string_view_literals;

std::unordered_map<Opcode, std::string_view> Get6502InstructionSet() {
    return {
        {INS_LDA_IM, "LDA_IM"sv},
        {INS_LDA_ZP, "LDA_ZP"sv},
        {INS_LDA_ZPX, "LDA_ZPX"sv},
        {INS_LDA_ABS, "LDA_ABS"sv},
        {INS_LDA_ABSX, "LDA_ABSX"sv},
        {INS_LDA_ABSY, "LDA_ABSY"sv},
        {INS_LDA_INDX, "LDA_INDX"sv},
        {INS_LDA_INDY, "LDA_INDY"sv},
        //LDX
        {INS_LDX_IM, "LDX_IM"sv},
        {INS_LDX_ZP, "LDX_ZP"sv},
        {INS_LDX_ZPY, "LDX_ZPY"sv},
        {INS_LDX_ABS, "LDX_ABS"sv},
        {INS_LDX_ABSY, "LDX_ABSY"sv},
        //LDY
        {INS_LDY_IM, "LDY_IM"sv},
        {INS_LDY_ZP, "LDY_ZP"sv},
        {INS_LDY_ZPX, "LDY_ZPX"sv},
        {INS_LDY_ABS, "LDY_ABS"sv},
        {INS_LDY_ABSX, "LDY_ABSX"sv},
        //STA
        {INS_STA_ZP, "STA_ZP"sv},
        {INS_STA_ZPX, "STA_ZPX"sv},
        {INS_STA_ABS, "STA_ABS"sv},
        {INS_STA_ABSX, "STA_ABSX"sv},
        {INS_STA_ABSY, "STA_ABSY"sv},
        {INS_STA_INDX, "STA_INDX"sv},
        {INS_STA_INDY, "STA_INDY"sv},
        //STX
        {INS_STX_ZP, "STX_ZP"sv},
        {INS_STX_ZPY, "STX_ZPY"sv},
        {INS_STX_ABS, "STX_ABS"sv},
        //STY
        {INS_STY_ZP, "STY_ZP"sv},
        {INS_STY_ZPX, "STY_ZPX"sv},
        {INS_STY_ABS, "STY_ABS"sv},

        {INS_TSX, "TSX"sv},
        {INS_TXS, "TXS"sv},
        {INS_PHA, "PHA"sv},
        {INS_PLA, "PLA"sv},
        {INS_PHP, "PHP"sv},
        {INS_PLP, "PLP"sv},

        {INS_JMP_ABS, "JMP_ABS"sv},
        {INS_JMP_IND, "JMP_IND"sv},
        {INS_JSR, "JSR"sv},
        {INS_RTS, "RTS"sv},

        //Logical Ops

        //AND
        {INS_AND_IM, "AND_IM"sv},
        {INS_AND_ZP, "AND_ZP"sv},
        {INS_AND_ZPX, "AND_ZPX"sv},
        {INS_AND_ABS, "AND_ABS"sv},
        {INS_AND_ABSX, "AND_ABSX"sv},
        {INS_AND_ABSY, "AND_ABSY"sv},
        {INS_AND_INDX, "AND_INDX"sv},
        {INS_AND_INDY, "AND_INDY"sv},

        //OR
        {INS_ORA_IM, "ORA_IM"sv},
        {INS_ORA_ZP, "ORA_ZP"sv},
        {INS_ORA_ZPX, "ORA_ZPX"sv},
        {INS_ORA_ABS, "ORA_ABS"sv},
        {INS_ORA_ABSX, "ORA_ABSX"sv},
        {INS_ORA_ABSY, "ORA_ABSY"sv},
        {INS_ORA_INDX, "ORA_INDX"sv},
        {INS_ORA_INDY, "ORA_INDY"sv},

        //EOR
        {INS_EOR_IM, "EOR_IM"sv},
        {INS_EOR_ZP, "EOR_ZP"sv},
        {INS_EOR_ZPX, "EOR_ZPX"sv},
        {INS_EOR_ABS, "EOR_ABS"sv},
        {INS_EOR_ABSX, "EOR_ABSX"sv},
        {INS_EOR_ABSY, "EOR_ABSY"sv},
        {INS_EOR_INDX, "EOR_INDX"sv},
        {INS_EOR_INDY, "EOR_INDY"sv},

        //BIT
        {INS_BIT_ZP, "BIT_ZP"sv},
        {INS_BIT_ABS, "BIT_ABS"sv},

        //Transfer Registers
        {INS_TAX, "TAX"sv},
        {INS_TAY, "TAY"sv},
        {INS_TXA, "TXA"sv},
        {INS_TYA, "TYA"sv},

        //Increments, Decrements
        {INS_INX, "INX"sv},
        {INS_INY, "INY"sv},
        {INS_DEY, "DEY"sv},
        {INS_DEX, "DEX"sv},
        {INS_DEC_ZP, "DEC_ZP"sv},
        {INS_DEC_ZPX, "DEC_ZPX"sv},
        {INS_DEC_ABS, "DEC_ABS"sv},
        {INS_DEC_ABSX, "DEC_ABSX"sv},
        {INS_INC_ZP, "INC_ZP"sv},
        {INS_INC_ZPX, "INC_ZPX"sv},
        {INS_INC_ABS, "INC_ABS"sv},
        {INS_INC_ABSX, "INC_ABSX"sv},

        //branches
        {INS_BEQ, "BEQ"sv},
        {INS_BNE, "BNE"sv},
        {INS_BCS, "BCS"sv},
        {INS_BCC, "BCC"sv},
        {INS_BMI, "BMI"sv},
        {INS_BPL, "BPL"sv},
        {INS_BVC, "BVC"sv},
        {INS_BVS, "BVS"sv},

        //status flag changes
        {INS_CLC, "CLC"sv},
        {INS_SEC, "SEC"sv},
        {INS_CLD, "CLD"sv},
        {INS_SED, "SED"sv},
        {INS_CLI, "CLI"sv},
        {INS_SEI, "SEI"sv},
        {INS_CLV, "CLV"sv},

        //Arithmetic
        {INS_ADC, "ADC"sv},
        {INS_ADC_ZP, "ADC_ZP"sv},
        {INS_ADC_ZPX, "ADC_ZPX"sv},
        {INS_ADC_ABS, "ADC_ABS"sv},
        {INS_ADC_ABSX, "ADC_ABSX"sv},
        {INS_ADC_ABSY, "ADC_ABSY"sv},
        {INS_ADC_INDX, "ADC_INDX"sv},
        {INS_ADC_INDY, "ADC_INDY"sv},

        {INS_SBC, "SBC"sv},
        {INS_SBC_ABS, "SBC_ABS"sv},
        {INS_SBC_ZP, "SBC_ZP"sv},
        {INS_SBC_ZPX, "SBC_ZPX"sv},
        {INS_SBC_ABSX, "SBC_ABSX"sv},
        {INS_SBC_ABSY, "SBC_ABSY"sv},
        {INS_SBC_INDX, "SBC_INDX"sv},
        {INS_SBC_INDY, "SBC_INDY"sv},

        // Register Comparison
        {INS_CMP, "CMP"sv},
        {INS_CMP_ZP, "CMP_ZP"sv},
        {INS_CMP_ZPX, "CMP_ZPX"sv},
        {INS_CMP_ABS, "CMP_ABS"sv},
        {INS_CMP_ABSX, "CMP_ABSX"sv},
        {INS_CMP_ABSY, "CMP_ABSY"sv},
        {INS_CMP_INDX, "CMP_INDX"sv},
        {INS_CMP_INDY, "CMP_INDY"sv},

        {INS_CPX, "CPX"sv},
        {INS_CPY, "CPY"sv},
        {INS_CPX_ZP, "CPX_ZP"sv},
        {INS_CPY_ZP, "CPY_ZP"sv},
        {INS_CPX_ABS, "CPX_ABS"sv},
        {INS_CPY_ABS, "CPY_ABS"sv},

        // shifts
        {INS_ASL, "ASL"sv},
        {INS_ASL_ZP, "ASL_ZP"sv},
        {INS_ASL_ZPX, "ASL_ZPX"sv},
        {INS_ASL_ABS, "ASL_ABS"sv},
        {INS_ASL_ABSX, "ASL_ABSX"sv},

        {INS_LSR, "LSR"sv},
        {INS_LSR_ZP, "LSR_ZP"sv},
        {INS_LSR_ZPX, "LSR_ZPX"sv},
        {INS_LSR_ABS, "LSR_ABS"sv},
        {INS_LSR_ABSX, "LSR_ABSX"sv},

        {INS_ROL, "ROL"sv},
        {INS_ROL_ZP, "ROL_ZP"sv},
        {INS_ROL_ZPX, "ROL_ZPX"sv},
        {INS_ROL_ABS, "ROL_ABS"sv},
        {INS_ROL_ABSX, "ROL_ABSX"sv},

        {INS_ROR, "ROR"sv},
        {INS_ROR_ZP, "ROR_ZP"sv},
        {INS_ROR_ZPX, "ROR_ZPX"sv},
        {INS_ROR_ABS, "ROR_ABS"sv},
        {INS_ROR_ABSX, "ROR_ABSX"sv},

        //misc
        {INS_NOP, "NOP"sv},
        {INS_BRK, "BRK"sv},
        {INS_RTI, "RTI"sv},
    };
}

} // namespace emu6502::cpu::opcode
