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
                } catch (const std::exception &e) {
                    std::cout << "EXCEPTION: " << e.what() << "\n";
                    throw;
                }
            },
            std::runtime_error);
    }
}

std::vector<AssemblerTestArg> GenTestCases() {
    std::unordered_map<std::string_view, std::unordered_map<AddressMode, std::tuple<OpcodeInfo, InstructionSet>>>
        instruction_set;

    std::vector<InstructionSet> instruction_sets{InstructionSet::NMOS6502Emu, InstructionSet::NMOS6502};
    for (auto is : instruction_sets) {
        for (auto &[opcode, info] : GetInstructionSet(is)) {
            instruction_set[info.mnemonic][info.addres_mode] = {info, is};
        }
    }

    struct TestVariant {
        std::string name;
        AddressMode mode;
        std::string test_string;
        std::vector<uint8_t> test_data;
        std::string relocation;
    };
    const std::vector<TestVariant> test_variants = {
        // | Immediate           |          #aa             |
        {"value", AddressMode::Immediate, "#$AA"s, {0xaa}, {}},
        {"alias_before", AddressMode::Immediate, "#CONST_BEFORE"s, {0x55}, {}},
        // {AddressMode::Immediate, "#CONST_AFTER"s, {}, {}},

        // | Implied             |                          |
        {"", AddressMode::Implied, ""s, {}, {}},

        // | Accumulator         |          A               |
        {"", AddressMode::ACC, "A"s, {}, {}},

        // | Absolute            |          aaaa            |
        {"value", AddressMode::ABS, "$1234"s, {0x34, 0x12}, {}},
        {"label_before", AddressMode::ABS, "LABEL_BEFORE"s, {0x00, 0x00}, "LABEL_BEFORE"s},
        {"label_after", AddressMode::ABS, "LABEL_AFTER"s, {0x20, 0x00}, "LABEL_AFTER"s},

        // | Relative            |          aaaa            |
        // {"value", AddressMode::REL, "$1234"s, {0x34, 0x12}, {}},
        {"label_before", AddressMode::REL, "LABEL_BEFORE"s, {0xed}, "LABEL_BEFORE"s},
        {"label_after", AddressMode::REL, "LABEL_AFTER"s, {0x0d}, "LABEL_AFTER"s},

        // | Indirect Absolute   |          (aaaa)          |
        {"value", AddressMode::ABS_IND, "($1234)"s, {0x34, 0x12}, {}},
        {"label_before", AddressMode::ABS_IND, "(LABEL_BEFORE)"s, {0x00, 0x00}, "LABEL_BEFORE"s},
        {"label_after", AddressMode::ABS_IND, "(LABEL_AFTER)"s, {0x20, 0x00}, "LABEL_AFTER"s},

        // | Zero Page           |          aa              |
        {"value", AddressMode::ZP, "$12"s, {0x12}, {}},
        {"alias_before", AddressMode::ZP, "CONST_BEFORE"s, {0x55}, {}},
        // {"value", AddressMode::ZP, "CONST_AFTER"s, {}, {}},

        // | Zero Page Indexed,X |          aa,X            |
        {"value", AddressMode::ZPX, "$12,X"s, {0x12}, {}},
        {"alias_before", AddressMode::ZPX, "CONST_BEFORE,X"s, {0x55}, {}},
        // {"value", AddressMode::ZPX, "CONST_AFTER,X"s, {}, {}},

        // | Zero Page Indexed,Y |          aa,Y            |
        {"value", AddressMode::ZPY, "$12,Y"s, {0x12}, {}},
        {"alias_before", AddressMode::ZPY, "CONST_BEFORE,Y"s, {0x55}, {}},
        // {"value", AddressMode::ZPY, "CONST_AFTER,Y"s, {}, {}},

        // | Absolute Indexed,X  |          aaaa,X          |
        {"value", AddressMode::ABSX, "$1234,X"s, {0x34, 0x12}, {}},
        {"label_before", AddressMode::ABSX, "LABEL_BEFORE,X"s, {0x00, 0x00}, "LABEL_BEFORE"s},
        {"label_after", AddressMode::ABSX, "LABEL_AFTER,X"s, {0x20, 0x00}, "LABEL_AFTER"s},

        // | Absolute Indexed,Y  |          aaaa,Y          |
        {"value", AddressMode::ABSY, "$1234,Y"s, {0x34, 0x12}, {}},
        {"label_before", AddressMode::ABSY, "LABEL_BEFORE,Y"s, {0x00, 0x00}, "LABEL_BEFORE"s},
        {"label_after", AddressMode::ABSY, "LABEL_AFTER,Y"s, {0x20, 0x00}, "LABEL_AFTER"s},

        // | Indexed Indirect    |          (aa,X)          |
        {"value", AddressMode::INDX, "($12,X)"s, {0x12}, {}},
        {"alias_before", AddressMode::INDX, "(CONST_BEFORE,X)"s, {0x55}, {}},
        // {"value", AddressMode::INDX, "(CONST_AFTER,X)"s, {}, {}},

        // | Indirect Indexed    |          (aa),Y          |
        {"value", AddressMode::INDY, "($12),Y"s, {0x12}, {}},
        {"alias_before", AddressMode::INDY, "(CONST_BEFORE),Y"s, {0x55}, {}},
        // {"value", AddressMode::INDY,  "(CONST_AFTER),Y"s, {}, {}},
    };

    const std::unordered_map<AddressMode, std::set<AddressMode>> kSkipScenarios = {
        {AddressMode::REL, {AddressMode::ABS, AddressMode::ABSX, AddressMode::ABSY}},
        {AddressMode::ABS, {AddressMode::REL}},
        {AddressMode::ABSX, {AddressMode::REL}},
        {AddressMode::ABSY, {AddressMode::REL}},
    };

    std::vector<AssemblerTestArg> test_cases;
    test_cases.reserve(instruction_set.size() * instruction_set.size());

    for (const auto &instruction : instruction_set) {
        for (const auto &variant : test_variants) {
            auto LABEL_BEFORE = std::make_shared<LabelInfo>(LabelInfo{"LABEL_BEFORE", 0_addr, false});
            auto LABEL = std::make_shared<LabelInfo>(LabelInfo{"LABEL", 0x0010_addr, false});
            auto LABEL_AFTER = std::make_shared<LabelInfo>(LabelInfo{"LABEL_AFTER", 0x0020_addr, false});

            auto CONST_BEFORE = std::make_shared<ValueAlias>(ValueAlias{"CONST_BEFORE", {0x55}});
            auto CONST_BEFORE_LONG = std::make_shared<ValueAlias>(ValueAlias{"CONST_BEFORE_LONG", {0x12, 0x34}});
            auto CONST_AFTER = std::make_shared<ValueAlias>(ValueAlias{"CONST_AFTER", {0xaa}});
            auto CONST_AFTER_LONG = std::make_shared<ValueAlias>(ValueAlias{"CONST_AFTER_LONG", {0x67, 0x89}});

            std::string name = fmt::format("{}_{}_{}", instruction.first, to_string(variant.mode), variant.name);
            if (name.back() == '_') {
                name.pop_back();
            }

            auto code_format = R"==(
CONST_BEFORE=$55
CONST_BEFORE_LONG=$1234

.org 0x0000
LABEL_BEFORE:
    NOP

.org 0x0010
LABEL:
    NOP
    {} {}
    NOP

.org 0x0020
LABEL_AFTER:
    NOP

CONST_AFTER=0xaa
CONST_AFTER_LONG=0x6789
)=="s;
            std::string code = fmt::format(code_format, instruction.first, variant.test_string);
            bool skip = false;
            if (auto skip_it = kSkipScenarios.find(variant.mode); skip_it != kSkipScenarios.end()) {
                for (auto item : skip_it->second) {
                    if (instruction.second.find(item) != instruction.second.end()) {
                        skip = true;
                        break;
                    }
                }
            }
            if (skip) {
                continue;
            }
            auto opcode = instruction.second.find(variant.mode);

            InstructionSet opcode_instruction_set = InstructionSet::Default;
            std::optional<Program> expected;
            if (opcode != instruction.second.end()) {
                auto [opcodeinfo, is] = opcode->second;
                opcode_instruction_set = is;
                SparseBinaryCode bin_code;
                bin_code.PutBytes(0x0000, {INS_NOP});
                bin_code.PutBytes(0x0010, {INS_NOP, opcodeinfo.opcode});
                bin_code.PutBytes(0x0012, variant.test_data);
                bin_code.PutBytes(static_cast<Address_t>(0x0012 + variant.test_data.size()), {INS_NOP});
                bin_code.PutBytes(0x0020, {INS_NOP});

                expected = Program{
                    .sparse_binary_code = bin_code,
                    .labels =
                        {
                            {"LABEL_BEFORE", LABEL_BEFORE},
                            {"LABEL", LABEL},
                            {"LABEL_AFTER", LABEL_AFTER},
                        },
                    .aliases =
                        {
                            {"CONST_BEFORE", CONST_BEFORE},
                            {"CONST_BEFORE_LONG", CONST_BEFORE_LONG},
                            {"CONST_AFTER", CONST_AFTER},
                            {"CONST_AFTER_LONG", CONST_AFTER_LONG},
                        },
                    .relocations = {},
                };
                if (!variant.relocation.empty()) {
                    auto l = expected->labels[variant.relocation];
                    auto ri = RelocationInfo{
                        .target_label = l,
                        .position = 0x0012,
                        .mode = variant.mode == AddressMode::REL ? RelocationMode::Relative : RelocationMode::Absolute,
                    };
                    auto sri = std::make_shared<RelocationInfo>(ri);
                    l->label_references.emplace_back(sri);
                    expected->relocations.insert(sri);
                }
            }

            test_cases.emplace_back(name, code, expected, opcode_instruction_set);
        }
    };
    return test_cases;
}

INSTANTIATE_TEST_SUITE_P(generated, CompilerTest, ::testing::ValuesIn(GenTestCases()),
                         [](auto &info) { return std::get<0>(info.param); });

} // namespace
} // namespace emu::emu6502::test