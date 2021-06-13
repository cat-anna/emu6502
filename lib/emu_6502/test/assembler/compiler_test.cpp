#include "emu_6502/assembler/compilation_error.hpp"
#include "emu_6502/assembler/compiler.hpp"
#include "emu_6502/cpu/opcode.hpp"
#include "emu_6502/instruction_set.hpp"
#include <emu_core/byte_utils.hpp>
#include <emu_core/program.hpp>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <string_view>
#include <tuple>
#include <vector>

namespace emu::emu6502::test {
namespace {

using namespace emu::emu6502::cpu::opcode;
using namespace emu::emu6502::assembler;
using namespace std::string_literals;
using namespace std::string_view_literals;

using AssemblerTestArg = std::tuple<std::string, std::string, std::optional<Program>, InstructionSet>;
class CompilerTest : public testing::TestWithParam<AssemblerTestArg> {};

TEST_P(CompilerTest, ) {
    auto &[name, code, expected, instruction_set] = GetParam();
    std::cout << "------------CODE----------------------\n" << code << "\n";

    if (expected.has_value()) {
        auto result = Compiler6502::CompileString(code, instruction_set);
        std::cout << "-----------RESULT---------------------\n"
                  << to_string(*result) << "\n"
                  << "----------EXPECTED--------------------\n"
                  << to_string(*expected) << "\n"
                  << "--------------------------------------\n";
        EXPECT_EQ(*expected, *result);
    } else {
        std::cout << "--------------------------------------\n";
        EXPECT_THROW(
            {
                try {
                    auto r = Compiler6502::CompileString(code, instruction_set);
                    std::cout << "-----------RESULT---------------------\n" << to_string(*r) << "\n";
                } catch (const CompilationException &e) {
                    std::cout << e.Message() << "\n";
                    throw;
                } catch (const std::exception &e) {
                    std::cout << "std::exception: " << e.what() << "\n";
                    throw;
                }
            },
            CompilationException);
    }
}

AssemblerTestArg GetJumpTest() {
    auto LABEL = std::make_shared<LabelInfo>(LabelInfo{"LABEL", 10_addr, false});
    Program expected = {
        .sparse_binary_code =
            SparseBinaryCode(1_addr, {0xaa, INS_JMP_ABS, 0x0a, 0x00, 0x55, INS_JMP_IND, 0x0a, 0x00, 0x55, INS_NOP}),
        .labels = {{"LABEL", LABEL}},
        .relocations = {std::make_shared<RelocationInfo>(RelocationInfo{LABEL, 3_addr, RelocationMode::Absolute}),
                        std::make_shared<RelocationInfo>(RelocationInfo{LABEL, 7_addr, RelocationMode::Absolute})},
    };
    auto code = R"==(
.org 0x01
.byte 0xaa
    JMP LABEL
.byte 0x55
    JMP (LABEL)
.byte 0x55
LABEL:
    NOP
)=="s;
    return {"jump_to_label", code, expected, InstructionSet::Default};
}

AssemblerTestArg GetAbsoluteAddressingTest() {
    Program expected = {
        .sparse_binary_code = SparseBinaryCode(0x1000_addr, {INS_DEC_ABS, 0xAA, 0x55, INS_INC_ABS, 0x55, 0xAA,
                                                             INS_LDA_ABSX, 0x00, 0x20, INS_LDA_ABSY, 0x00, 0x20}),
        .labels = {},
        .relocations = {},
    };
    auto code = R"==(
.org 0x1000
    DEC $55AA
    INC $AA55
    LDA $2000,X
    LDA $2000,Y
)=="s;
    return {"absolute_addressing", code, expected, InstructionSet::Default};
}

AssemblerTestArg GetImpliedTest() {
    Program expected = {
        .sparse_binary_code = SparseBinaryCode({INS_NOP, INS_INY, INS_INX}),
        .labels = {},
        .relocations = {},
    };
    auto code = R"==(
NOP
INY
INX
)=="s;
    return {"implied", code, expected, InstructionSet::Default};
}

AssemblerTestArg GetOriginCommandTest() {
    Program expected = {
        .sparse_binary_code = SparseBinaryCode(0xE_addr, {0xaa, 55, 0x22, 0x11, 0x33, 0x22}),
        .labels = {},
        .relocations = {},
    };
    auto code = R"==(
.org 0x0010
.word 0x1122
.org 0x12
.word 0x2233
.org 0x0E
.byte 0xAA
.byte 55
)=="s;
    return {"origin", code, expected, InstructionSet::Default};
}

AssemblerTestArg GetTextCommandTest() {
    Program expected = {
        .sparse_binary_code = SparseBinaryCode(0x0100_addr, ToBytes("abcd\n\0"sv)),
        .labels = {},
        .relocations = {},
    };
    auto code = R"==(
.org 0x0100
.text "abcd\n"
)=="s;
    return {"text", code, expected, InstructionSet::Default};
}

AssemblerTestArg GetPageAlignCommandTest() {
    Program expected = {
        .sparse_binary_code = SparseBinaryCode({{0_addr, 1_u8}, {0x100_addr, 2_u8}, {0x200_addr, 3_u8}}),
        .labels = {},
        .relocations = {},
    };
    auto code = R"==(
.byte 1
.page_align
.byte 2
.page_align
.byte 3
)=="s;
    return {"page_align", code, expected, InstructionSet::Default};
}

AssemblerTestArg GetBranchTest() {
    auto L1 = std::make_shared<LabelInfo>(LabelInfo{"L1", 1_addr, false});
    auto L2 = std::make_shared<LabelInfo>(LabelInfo{"L2", 6_addr, false});
    Program expected = {
        .sparse_binary_code = SparseBinaryCode({INS_NOP, INS_BEQ, 0x03_u8, INS_NOP, INS_BPL, 0xfb_u8, INS_NOP}),
        .labels = {{"L1", L1}, {"L2", L2}},
        .relocations =
            {
                std::make_shared<RelocationInfo>(RelocationInfo{L2, 2_addr, RelocationMode::Relative}),
                std::make_shared<RelocationInfo>(RelocationInfo{L1, 5_addr, RelocationMode::Relative}),
            },
    };
    auto code = R"==(
    NOP
L1:
    BEQ L2
    NOP
    BPL L1
L2:
    NOP
)=="s;
    return {"branch", code, expected, InstructionSet::Default};
}

AssemblerTestArg GetImmediateTest() {
    Program expected = {
        .sparse_binary_code = SparseBinaryCode({INS_AND_IM, 0xdd_u8, INS_ORA_IM, 0xaa_u8, INS_EOR_IM, 0xff_u8}),
        .labels = {},
        .relocations = {},
    };
    auto code = R"==(
AND #$DD
ORA #$AA
EOR #$ff
)=="s;
    return {"immediate", code, expected, InstructionSet::Default};
}

AssemblerTestArg GetZPTest() {
    Program expected = {
        .sparse_binary_code =
            SparseBinaryCode({INS_AND_ZP, 0xdd_u8, INS_LDY_ZP, 0xaa, INS_LDY_ZPX, 0xaa_u8, INS_LDX_ZPY, 0xff_u8}),
        .labels = {},
        .relocations = {},
    };
    auto code = R"==(
AND $DD
LDY $aa
LDY $AA,X
LDX $ff,Y
)=="s;
    return {"zp", code, expected, InstructionSet::Default};
}

AssemblerTestArg GetAliasTest() {
    auto ALIAS = std::make_shared<ValueAlias>(ValueAlias{"ALIAS", {0x10_u8}});
    Program expected = {
        .sparse_binary_code = SparseBinaryCode(0_addr, {0x10, INS_LDA_ZP, 0x10, INS_LDA_IM, 0x10}),
        .aliases = {{"ALIAS", ALIAS}},
    };
    auto code = R"==(
ALIAS=0x10
.byte ALIAS
LDA ALIAS
LDA #ALIAS
)=="s;
    return {"alias", code, expected, InstructionSet::Default};
}

INSTANTIATE_TEST_SUITE_P(positive, CompilerTest,
                         ::testing::ValuesIn({
                             GetAbsoluteAddressingTest(),
                             GetImpliedTest(),
                             GetPageAlignCommandTest(),
                             GetOriginCommandTest(),
                             GetTextCommandTest(),
                             GetJumpTest(),
                             GetBranchTest(),
                             GetImmediateTest(),
                             GetZPTest(),
                             GetAliasTest(),
                         }),
                         [](auto &info) { return std::get<0>(info.param); });

INSTANTIATE_TEST_SUITE_P(
    negative, CompilerTest,
    ::testing::ValuesIn({
        AssemblerTestArg{"duplicated_label", "\nLABEL:\n.byte 0x00\nLABEL:", std::nullopt, InstructionSet::Default},
        AssemblerTestArg{"duplicated_alias", "ALIAS=0x00\nALIAS equ 0x00\n", std::nullopt, InstructionSet::Default},
        AssemblerTestArg{"invalid_abs_ind_mode", "INC ($1234)", std::nullopt, InstructionSet::Default},
    }),
    [](auto &info) { return std::get<0>(info.param); });

} // namespace
} // namespace emu::emu6502::test