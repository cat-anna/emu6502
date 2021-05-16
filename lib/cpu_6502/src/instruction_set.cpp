#include "cpu_6502/instruction_set.hpp"
#include "cpu_6502/opcode.hpp"
#include <fmt/format.h>
#include <stdexcept>

namespace emu::cpu6502 {

namespace {

using namespace std::string_view_literals;
using namespace opcode;

void MergeInstructionMap(OpcodeInstructionMap &out, OpcodeInstructionMap source) {
    out.merge(source);
    if (!source.empty()) {
        throw std::runtime_error("Instruction opcode is duplicated!");
    }
}

OpcodeInstructionMap GenLoadInstructions(InstructionSet instruction_set) {
    return {
        //LDA
        {INS_LDA_IM, {INS_LDA_IM, "LDA"sv, AddressMode::Immediate}},
        {INS_LDA_ZP, {INS_LDA_ZP, "LDA"sv, AddressMode::ZP}},
        {INS_LDA_ZPX, {INS_LDA_ZPX, "LDA"sv, AddressMode::ZPX}},
        {INS_LDA_ABS, {INS_LDA_ABS, "LDA"sv, AddressMode::ABS}},
        {INS_LDA_ABSX, {INS_LDA_ABSX, "LDA"sv, AddressMode::ABSX}},
        {INS_LDA_ABSY, {INS_LDA_ABSY, "LDA"sv, AddressMode::ABSY}},
        {INS_LDA_INDX, {INS_LDA_INDX, "LDA"sv, AddressMode::INDX}},
        {INS_LDA_INDY, {INS_LDA_INDY, "LDA"sv, AddressMode::INDY}},
        //LDX
        {INS_LDX_IM, {INS_LDX_IM, "LDX"sv, AddressMode::Immediate}},
        {INS_LDX_ZP, {INS_LDX_ZP, "LDX"sv, AddressMode::ZP}},
        {INS_LDX_ZPY, {INS_LDX_ZPY, "LDX"sv, AddressMode::ZPY}},
        {INS_LDX_ABS, {INS_LDX_ABS, "LDX"sv, AddressMode::ABS}},
        {INS_LDX_ABSY, {INS_LDX_ABSY, "LDX"sv, AddressMode::ABSY}},
        //LDY
        {INS_LDY_IM, {INS_LDY_IM, "LDY"sv, AddressMode::Immediate}},
        {INS_LDY_ZP, {INS_LDY_ZP, "LDY"sv, AddressMode::ZP}},
        {INS_LDY_ZPX, {INS_LDY_ZPX, "LDY"sv, AddressMode::ZPX}},
        {INS_LDY_ABS, {INS_LDY_ABS, "LDY"sv, AddressMode::ABS}},
        {INS_LDY_ABSX, {INS_LDY_ABSX, "LDY"sv, AddressMode::ABSX}},
    };
}

OpcodeInstructionMap GenStoreInstructions(InstructionSet instruction_set) {
    return {
        //STA
        {INS_STA_ZP, {INS_STA_ZP, "STA"sv, AddressMode::ZP}},
        {INS_STA_ZPX, {INS_STA_ZPX, "STA"sv, AddressMode::ZPX}},
        {INS_STA_ABS, {INS_STA_ABS, "STA"sv, AddressMode::ABS}},
        {INS_STA_ABSX, {INS_STA_ABSX, "STA"sv, AddressMode::ABSX}},
        {INS_STA_ABSY, {INS_STA_ABSY, "STA"sv, AddressMode::ABSY}},
        {INS_STA_INDX, {INS_STA_INDX, "STA"sv, AddressMode::INDX}},
        {INS_STA_INDY, {INS_STA_INDY, "STA"sv, AddressMode::INDY}},
        //STX
        {INS_STX_ZP, {INS_STX_ZP, "STX"sv, AddressMode::ZP}},
        {INS_STX_ZPY, {INS_STX_ZPY, "STX"sv, AddressMode::ZPY}},
        {INS_STX_ABS, {INS_STX_ABS, "STX"sv, AddressMode::ABS}},
        //STY
        {INS_STY_ZP, {INS_STY_ZP, "STY"sv, AddressMode::ZP}},
        {INS_STY_ZPX, {INS_STY_ZPX, "STY"sv, AddressMode::ZPX}},
        {INS_STY_ABS, {INS_STY_ABS, "STY"sv, AddressMode::ABS}},
    };
}

OpcodeInstructionMap GenLogicalInstructions(InstructionSet instruction_set) {
    return {
        //AND
        {INS_AND_IM, {INS_AND_IM, "AND"sv, AddressMode::Immediate}},
        {INS_AND_ZP, {INS_AND_ZP, "AND"sv, AddressMode::ZP}},
        {INS_AND_ZPX, {INS_AND_ZPX, "AND"sv, AddressMode::ZPX}},
        {INS_AND_ABS, {INS_AND_ABS, "AND"sv, AddressMode::ABS}},
        {INS_AND_ABSX, {INS_AND_ABSX, "AND"sv, AddressMode::ABSX}},
        {INS_AND_ABSY, {INS_AND_ABSY, "AND"sv, AddressMode::ABSY}},
        {INS_AND_INDX, {INS_AND_INDX, "AND"sv, AddressMode::INDX}},
        {INS_AND_INDY, {INS_AND_INDY, "AND"sv, AddressMode::INDY}},
        //OR
        {INS_ORA_IM, {INS_ORA_IM, "ORA"sv, AddressMode::Immediate}},
        {INS_ORA_ZP, {INS_ORA_ZP, "ORA"sv, AddressMode::ZP}},
        {INS_ORA_ZPX, {INS_ORA_ZPX, "ORA"sv, AddressMode::ZPX}},
        {INS_ORA_ABS, {INS_ORA_ABS, "ORA"sv, AddressMode::ABS}},
        {INS_ORA_ABSX, {INS_ORA_ABSX, "ORA"sv, AddressMode::ABSX}},
        {INS_ORA_ABSY, {INS_ORA_ABSY, "ORA"sv, AddressMode::ABSY}},
        {INS_ORA_INDX, {INS_ORA_INDX, "ORA"sv, AddressMode::INDX}},
        {INS_ORA_INDY, {INS_ORA_INDY, "ORA"sv, AddressMode::INDY}},
        //EOR
        {INS_EOR_IM, {INS_EOR_IM, "EOR"sv, AddressMode::Immediate}},
        {INS_EOR_ZP, {INS_EOR_ZP, "EOR"sv, AddressMode::ZP}},
        {INS_EOR_ZPX, {INS_EOR_ZPX, "EOR"sv, AddressMode::ZPX}},
        {INS_EOR_ABS, {INS_EOR_ABS, "EOR"sv, AddressMode::ABS}},
        {INS_EOR_ABSX, {INS_EOR_ABSX, "EOR"sv, AddressMode::ABSX}},
        {INS_EOR_ABSY, {INS_EOR_ABSY, "EOR"sv, AddressMode::ABSY}},
        {INS_EOR_INDX, {INS_EOR_INDX, "EOR"sv, AddressMode::INDX}},
        {INS_EOR_INDY, {INS_EOR_INDY, "EOR"sv, AddressMode::INDY}},
    };
}

OpcodeInstructionMap GenArithmeticInstructions(InstructionSet instruction_set) {
    return {
        //Addition
        {INS_ADC, {INS_ADC, "ADC"sv, AddressMode::Immediate}},
        {INS_ADC_ZP, {INS_ADC_ZP, "ADC"sv, AddressMode::ZP}},
        {INS_ADC_ZPX, {INS_ADC_ZPX, "ADC"sv, AddressMode::ZPX}},
        {INS_ADC_ABS, {INS_ADC_ABS, "ADC"sv, AddressMode::ABS}},
        {INS_ADC_ABSX, {INS_ADC_ABSX, "ADC"sv, AddressMode::ABSX}},
        {INS_ADC_ABSY, {INS_ADC_ABSY, "ADC"sv, AddressMode::ABSY}},
        {INS_ADC_INDX, {INS_ADC_INDX, "ADC"sv, AddressMode::INDX}},
        {INS_ADC_INDY, {INS_ADC_INDY, "ADC"sv, AddressMode::INDY}},
        //Subtraction
        {INS_SBC, {INS_SBC, "SBC"sv, AddressMode::Immediate}},
        {INS_SBC_ABS, {INS_SBC_ABS, "SBC"sv, AddressMode::ABS}},
        {INS_SBC_ZP, {INS_SBC_ZP, "SBC"sv, AddressMode::ZP}},
        {INS_SBC_ZPX, {INS_SBC_ZPX, "SBC"sv, AddressMode::ZPX}},
        {INS_SBC_ABSX, {INS_SBC_ABSX, "SBC"sv, AddressMode::ABSX}},
        {INS_SBC_ABSY, {INS_SBC_ABSY, "SBC"sv, AddressMode::ABSY}},
        {INS_SBC_INDX, {INS_SBC_INDX, "SBC"sv, AddressMode::INDX}},
        {INS_SBC_INDY, {INS_SBC_INDY, "SBC"sv, AddressMode::INDY}},
    };
}

OpcodeInstructionMap GenShiftsInstructions(InstructionSet instruction_set) {
    return {
        // shifts
        {INS_ASL, {INS_ASL, "ASL"sv, AddressMode::Immediate}},
        {INS_ASL_ZP, {INS_ASL_ZP, "ASL"sv, AddressMode::ZP}},
        {INS_ASL_ZPX, {INS_ASL_ZPX, "ASL"sv, AddressMode::ZPX}},
        {INS_ASL_ABS, {INS_ASL_ABS, "ASL"sv, AddressMode::ABS}},
        {INS_ASL_ABSX, {INS_ASL_ABSX, "ASL"sv, AddressMode::ABSX}},

        {INS_LSR, {INS_LSR, "LSR"sv, AddressMode::Immediate}},
        {INS_LSR_ZP, {INS_LSR_ZP, "LSR"sv, AddressMode::ZP}},
        {INS_LSR_ZPX, {INS_LSR_ZPX, "LSR"sv, AddressMode::ZPX}},
        {INS_LSR_ABS, {INS_LSR_ABS, "LSR"sv, AddressMode::ABS}},
        {INS_LSR_ABSX, {INS_LSR_ABSX, "LSR"sv, AddressMode::ABSX}},

        {INS_ROL, {INS_ROL, "ROL"sv, AddressMode::Immediate}},
        {INS_ROL_ZP, {INS_ROL_ZP, "ROL"sv, AddressMode::ZP}},
        {INS_ROL_ZPX, {INS_ROL_ZPX, "ROL"sv, AddressMode::ZPX}},
        {INS_ROL_ABS, {INS_ROL_ABS, "ROL"sv, AddressMode::ABS}},
        {INS_ROL_ABSX, {INS_ROL_ABSX, "ROL"sv, AddressMode::ABSX}},

        {INS_ROR, {INS_ROR, "ROR"sv, AddressMode::Immediate}},
        {INS_ROR_ZP, {INS_ROR_ZP, "ROR"sv, AddressMode::ZP}},
        {INS_ROR_ZPX, {INS_ROR_ZPX, "ROR"sv, AddressMode::ZPX}},
        {INS_ROR_ABS, {INS_ROR_ABS, "ROR"sv, AddressMode::ABS}},
        {INS_ROR_ABSX, {INS_ROR_ABSX, "ROR"sv, AddressMode::ABSX}},
    };
}

OpcodeInstructionMap GenIncrementsInstructions(InstructionSet instruction_set) {
    return {
        //Increments, Decrements
        {INS_INX, {INS_INX, "INX"sv, AddressMode::Implied}},
        {INS_INY, {INS_INY, "INY"sv, AddressMode::Implied}},
        {INS_DEY, {INS_DEY, "DEY"sv, AddressMode::Implied}},
        {INS_DEX, {INS_DEX, "DEX"sv, AddressMode::Implied}},
        {INS_DEC_ZP, {INS_DEC_ZP, "DEC"sv, AddressMode::ZP}},
        {INS_DEC_ZPX, {INS_DEC_ZPX, "DEC"sv, AddressMode::ZPX}},
        {INS_DEC_ABS, {INS_DEC_ABS, "DEC"sv, AddressMode::ABS}},
        {INS_DEC_ABSX, {INS_DEC_ABSX, "DEC"sv, AddressMode::ABSX}},
        {INS_INC_ZP, {INS_INC_ZP, "INC"sv, AddressMode::ZP}},
        {INS_INC_ZPX, {INS_INC_ZPX, "INC"sv, AddressMode::ZPX}},
        {INS_INC_ABS, {INS_INC_ABS, "INC"sv, AddressMode::ABS}},
        {INS_INC_ABSX, {INS_INC_ABSX, "INC"sv, AddressMode::ABSX}},
    };
}

OpcodeInstructionMap GenComparisionInstructions(InstructionSet instruction_set) {
    return {
        // Register Comparison
        {INS_CMP, {INS_CMP, "CMP"sv, AddressMode::Immediate}},
        {INS_CMP_ZP, {INS_CMP_ZP, "CMP"sv, AddressMode::ZP}},
        {INS_CMP_ZPX, {INS_CMP_ZPX, "CMP"sv, AddressMode::ZPX}},
        {INS_CMP_ABS, {INS_CMP_ABS, "CMP"sv, AddressMode::ABS}},
        {INS_CMP_ABSX, {INS_CMP_ABSX, "CMP"sv, AddressMode::ABSX}},
        {INS_CMP_ABSY, {INS_CMP_ABSY, "CMP"sv, AddressMode::ABSY}},
        {INS_CMP_INDX, {INS_CMP_INDX, "CMP"sv, AddressMode::INDX}},
        {INS_CMP_INDY, {INS_CMP_INDY, "CMP"sv, AddressMode::INDY}},

        {INS_CPX, {INS_CPX, "CPX"sv, AddressMode::Immediate}},
        {INS_CPX_ZP, {INS_CPX_ZP, "CPX"sv, AddressMode::ZP}},
        {INS_CPX_ABS, {INS_CPX_ABS, "CPX"sv, AddressMode::ABS}},

        {INS_CPY, {INS_CPY, "CPY"sv, AddressMode::Immediate}},
        {INS_CPY_ZP, {INS_CPY_ZP, "CPY"sv, AddressMode::ZP}},
        {INS_CPY_ABS, {INS_CPY_ABS, "CPY"sv, AddressMode::ABS}},
    };
}

OpcodeInstructionMap GenJumpsInstructions(InstructionSet instruction_set) {
    return {
        //branches
        {INS_BEQ, {INS_BEQ, "BEQ"sv, AddressMode::REL}},
        {INS_BNE, {INS_BNE, "BNE"sv, AddressMode::REL}},
        {INS_BCS, {INS_BCS, "BCS"sv, AddressMode::REL}},
        {INS_BCC, {INS_BCC, "BCC"sv, AddressMode::REL}},
        {INS_BMI, {INS_BMI, "BMI"sv, AddressMode::REL}},
        {INS_BPL, {INS_BPL, "BPL"sv, AddressMode::REL}},
        {INS_BVC, {INS_BVC, "BVC"sv, AddressMode::REL}},
        {INS_BVS, {INS_BVS, "BVS"sv, AddressMode::REL}},

        //Jumps
        {INS_JMP_ABS, {INS_JMP_ABS, "JMP"sv, AddressMode::ABS}},
        {INS_JMP_IND, {INS_JMP_IND, "JMP"sv, AddressMode::ABS_IND}},

        //Call/Return subroutine
        {INS_JSR, {INS_JSR, "JSR"sv, AddressMode::Immediate}},
        {INS_RTS, {INS_RTS, "RTS"sv, AddressMode::Immediate}},

    };
}

OpcodeInstructionMap GenStatusFlagsInstructions(InstructionSet instruction_set) {
    return {
        //status flag changes
        {INS_CLC, {INS_CLC, "CLC"sv, AddressMode::Implied}}, //
        {INS_SEC, {INS_SEC, "SEC"sv, AddressMode::Implied}}, //
        {INS_CLD, {INS_CLD, "CLD"sv, AddressMode::Implied}}, //
        {INS_SED, {INS_SED, "SED"sv, AddressMode::Implied}}, //
        {INS_CLI, {INS_CLI, "CLI"sv, AddressMode::Implied}}, //
        {INS_SEI, {INS_SEI, "SEI"sv, AddressMode::Implied}}, //
        {INS_CLV, {INS_CLV, "CLV"sv, AddressMode::Implied}}, //
    };
}

OpcodeInstructionMap GenStackInstructions(InstructionSet instruction_set) {
    return {
        {INS_TSX, {INS_TSX, "TSX"sv, AddressMode::Implied}}, //
        {INS_TXS, {INS_TXS, "TXS"sv, AddressMode::Implied}}, //
        {INS_PHA, {INS_PHA, "PHA"sv, AddressMode::Implied}}, //
        {INS_PLA, {INS_PLA, "PLA"sv, AddressMode::Implied}}, //
        {INS_PHP, {INS_PHP, "PHP"sv, AddressMode::Implied}}, //
        {INS_PLP, {INS_PLP, "PLP"sv, AddressMode::Implied}}, //
    };
}

OpcodeInstructionMap GenTransferInstructions(InstructionSet instruction_set) {
    return {
        {INS_TAX, {INS_TAX, "TAX"sv, AddressMode::Implied}},
        {INS_TAY, {INS_TAY, "TAY"sv, AddressMode::Implied}},
        {INS_TXA, {INS_TXA, "TXA"sv, AddressMode::Implied}},
        {INS_TYA, {INS_TYA, "TYA"sv, AddressMode::Implied}},

    };
}

OpcodeInstructionMap GenBitInstructions(InstructionSet instruction_set) {
    return {
        {INS_BIT_ZP, {INS_BIT_ZP, "BIT"sv, AddressMode::ZP}},
        {INS_BIT_ABS, {INS_BIT_ABS, "BIT"sv, AddressMode::ABS}},
    };
}

OpcodeInstructionMap GenInterruptInstructions(InstructionSet instruction_set) {
    return {
        //Return from interrupt
        {INS_RTI, {INS_RTI, "RTI"sv, AddressMode::Implied}},   //
        {INS_BRK, {INS_BRK, "BRK"sv, AddressMode::Immediate}}, //
    };
}

OpcodeInstructionMap GenMiscInstructions(InstructionSet instruction_set) {
    return {
        {INS_NOP, {INS_NOP, "NOP"sv, AddressMode::Implied}}, //
    };
}

// OpcodeInstructionMap GenArithmeticInstructions(InstructionSet instruction_set) {
//     return {
//         //
//     };
// }

OpcodeInstructionMap GenerateInstructionSet(InstructionSet instruction_set) {
    OpcodeInstructionMap r;
    MergeInstructionMap(r, GenLoadInstructions(instruction_set));
    MergeInstructionMap(r, GenStoreInstructions(instruction_set));
    MergeInstructionMap(r, GenLogicalInstructions(instruction_set));
    MergeInstructionMap(r, GenArithmeticInstructions(instruction_set));
    MergeInstructionMap(r, GenShiftsInstructions(instruction_set));
    MergeInstructionMap(r, GenIncrementsInstructions(instruction_set));
    MergeInstructionMap(r, GenComparisionInstructions(instruction_set));
    MergeInstructionMap(r, GenJumpsInstructions(instruction_set));
    MergeInstructionMap(r, GenStatusFlagsInstructions(instruction_set));
    MergeInstructionMap(r, GenStackInstructions(instruction_set));
    MergeInstructionMap(r, GenTransferInstructions(instruction_set));
    MergeInstructionMap(r, GenBitInstructions(instruction_set));
    MergeInstructionMap(r, GenInterruptInstructions(instruction_set));
    MergeInstructionMap(r, GenMiscInstructions(instruction_set));
    return r;
}

} // namespace

std::string to_string(AddressMode mode) {
    switch (mode) {
    case AddressMode::IM:
        return "Immediate";
    case AddressMode::ABS:
        return "ABS";
    case AddressMode::ZP:
        return "ZP";
    case AddressMode::ZPX:
        return "ZPX";
    case AddressMode::ZPY:
        return "ZPY";
    case AddressMode::ABSX:
        return "ABSX";
    case AddressMode::ABSY:
        return "ABSY";
    case AddressMode::INDY:
        return "INDY";
    case AddressMode::INDX:
        return "INDX";
    case AddressMode::ACC:
        return "ACC";
    case AddressMode::REL:
        return "REL";
    case AddressMode::Implied:
        return "Implied";
    case AddressMode::ABS_IND:
        return "ABS_IND";
    }
    throw std::runtime_error(fmt::format("Invalid address mode: {}", static_cast<int>(mode)));
}

size_t ArgumentByteSize(AddressMode mode) {
    switch (mode) {
    case AddressMode::ACC:
    case AddressMode::Implied:
        return 0;
    case AddressMode::IM:
    case AddressMode::ZP:
    case AddressMode::ZPX:
    case AddressMode::ZPY:
    case AddressMode::INDY:
    case AddressMode::INDX:
    case AddressMode::REL:
        return 1;
    case AddressMode::ABS:
    case AddressMode::ABSX:
    case AddressMode::ABSY:
    case AddressMode::ABS_IND:
        return 2;
    }
    throw std::runtime_error(fmt::format("Invalid address mode: {}", static_cast<int>(mode)));
}

const OpcodeInstructionMap &GetInstructionSet(InstructionSet instruction_set) {
    switch (instruction_set) {
    case InstructionSet::NMOS6502:
        return Get6502InstructionSet();
    }
    throw std::runtime_error(fmt::format("Invalid instruction set: {}", static_cast<int>(instruction_set)));
}

const OpcodeInstructionMap &Get6502InstructionSet() {
    static auto instruction_set = GenerateInstructionSet(InstructionSet::NMOS6502);
    return instruction_set;
}

} // namespace emu::cpu6502
