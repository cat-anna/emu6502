#include <assembler/assember.hpp>
#include <cpu/opcode.hpp>
#include <gtest/gtest.h>
#include <string_view>
#include <vector>

using namespace emu6502::cpu::opcode;
using namespace emu6502::assembler;
using namespace std::string_literals;

using AssemblerTestArg = std::tuple<std::string, std::string, Program>;
class BaseTest : public testing::TestWithParam<AssemblerTestArg> {
public:
};

TEST_P(BaseTest, ) {
    auto &[name, code, expected] = GetParam();
    auto result = CompileString(code);
    EXPECT_EQ(expected, *result) << "----------EXPECTED--------------------\n"
                                 << to_string(expected) << "\n"
                                 << "-----------RESULT---------------------\n"
                                 << to_string(*result) << "\n"
                                 << "--------------------------------------\n";
}

AssemblerTestArg GetJumpTest() {
    auto LABEL = std::make_shared<LabelInfo>(LabelInfo{"LABEL", 6, false});
    Program expected = {
        .sparse_binary_code = SparseBinaryCode(1, {0xaa, INS_JMP_ABS, 0x06, 0x00, 0x55, INS_NOP}),
        .labels = {{"LABEL", LABEL}},
        .relocations = {std::make_shared<RelocationInfo>(LABEL, 3, RelocationMode::Absolute)},
    };
    auto code = R"==(
.org 0x01
.byte 0xaa
    JMP LABEL
.byte 0x55
LABEL:
    NOP
)=="s;
    return {"jump_to_label", code, expected};
}

AssemblerTestArg GetAbsoluteAddressingTest() {
    auto LABEL = std::make_shared<LabelInfo>(LabelInfo{"LABEL", 6, false});
    Program expected = {
        .sparse_binary_code = SparseBinaryCode(0x1000, {INS_DEC_ABS, 0xAA, 0x55, INS_INC_ABS, 0x55, 0xAA}),
        .labels = {},
        .relocations = {},
    };
    auto code = R"==(
.org 0x1000
DEC $55AA
INC $AA55
)=="s;
    return {"absolute_addressing", code, expected};
}

AssemblerTestArg GetImpliedTest() {
    auto LABEL = std::make_shared<LabelInfo>(LabelInfo{"LABEL", 6, false});
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
    return {"implied", code, expected};
}

AssemblerTestArg GetOriginCommandTest() {
    auto LABEL = std::make_shared<LabelInfo>(LabelInfo{"LABEL", 6, false});
    Program expected = {
        .sparse_binary_code = SparseBinaryCode(0xE, {0xaa, 55, 0x22, 0x11, 0x33, 0x22}),
        .labels = {},
        .relocations = {},
    };
    auto code = R"==(
.org 0x10
.word 0x1122
.org 0x12
.word 0x2233
.org 0x0E
.byte 0xAA
.byte 55
)=="s;
    return {"org_command", code, expected};
}

AssemblerTestArg GetPageAlignCommandTest() {
    auto LABEL = std::make_shared<LabelInfo>(LabelInfo{"LABEL", 6, false});
    Program expected = {
        .sparse_binary_code = SparseBinaryCode({{0, 1}, {0x100, 2}, {0x200, 3}}),
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
    return {"page_align", code, expected};
}

INSTANTIATE_TEST_SUITE_P(, BaseTest,
                         ::testing::ValuesIn({
                             GetAbsoluteAddressingTest(),
                             GetImpliedTest(),
                             GetPageAlignCommandTest(),
                             GetOriginCommandTest(),
                             GetJumpTest(),
                         }),
                         [](auto &info) { return std::get<0>(info.param); });

TEST_F(BaseTest, duplicated_label) {
    EXPECT_THROW(CompileString("\nLABEL:\n.byte 0x00\nLABEL:"), std::runtime_error);
}
