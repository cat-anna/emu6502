#include "emu_6502/assembler/compilation_error.hpp"
#include "emu_6502/assembler/compiler.hpp"
#include "emu_6502/cpu/opcode.hpp"
#include "emu_6502/instruction_set.hpp"
#include <emu_core/byte_utils.hpp>
#include <emu_core/program.hpp>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <optional>
#include <string_view>
#include <tuple>
#include <vector>

namespace emu::emu6502::test {
namespace {

using namespace emu::emu6502::cpu::opcode;
using namespace emu::emu6502::assembler;
using namespace std::string_literals;
using namespace std::string_view_literals;

using AssemblerTestArg =
    std::tuple<std::string, std::string, std::optional<Program>, InstructionSet>;
class CompilerTest : public testing::TestWithParam<AssemblerTestArg> {};
using u8v = std::vector<uint8_t>;

TEST_P(CompilerTest, ) {
    auto &[name, code, expected, instruction_set] = GetParam();
    std::cout << "------------CODE----------------------\n" << code << "\n";

    if (expected.has_value()) {
        auto result = CompileString(code, instruction_set);
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
                    auto r = CompileString(code, instruction_set);
                    std::cout << "-----------RESULT---------------------\n"
                              << to_string(*r) << "\n";
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

std::vector<AssemblerTestArg> GenTestCases(AddressMode filter) {
    std::unordered_map<
        std::string_view,
        std::unordered_map<AddressMode, std::tuple<OpcodeInfo, InstructionSet>>>
        instruction_set;

    std::vector<InstructionSet> instruction_sets{InstructionSet::NMOS6502Emu,
                                                 InstructionSet::NMOS6502};
    for (auto is : instruction_sets) {
        for (auto &[opcode, info] : GetInstructionSet(is)) {
            instruction_set[info.mnemonic][info.addres_mode] = {info, is};
        }
    }

    struct TestVariant {
        std::string name;
        AddressMode mode;
        std::string test_string;
        std::optional<std::vector<uint8_t>> test_data;
        std::string relocation;
    };
    const std::vector<TestVariant> test_variants = {
        // clang-format off
        // | Immediate           |          #aa             |
        {"value", AddressMode::Immediate, "#$AA"s, u8v{0xaa}, {}},
        {"label_before", AddressMode::Immediate, "#LABEL_BEFORE,X"s, std::nullopt, {}},
        {"label_after", AddressMode::Immediate, "#LABEL_AFTER,X"s, std::nullopt, {}},
        {"alias_before", AddressMode::Immediate, "#ALIAS_BEFORE"s, u8v{0x55}, {}},
        {"alias_before_long", AddressMode::Immediate, "#ALIAS_BEFORE_LONG"s, std::nullopt, {}},

        // | Implied             |                          |
        {"", AddressMode::Implied, ""s, u8v{}, {}},

        // | Accumulator         |          A               |
        {"", AddressMode::ACC, "A"s, u8v{}, {}},

        // | Absolute            |          aaaa            |
        {"value", AddressMode::ABS, "$1234"s, u8v{0x34, 0x12}, {}},
        {"label_before", AddressMode::ABS, "LABEL_BEFORE"s, u8v{0x01, 0x00}, "LABEL_BEFORE"s},
        {"label_after", AddressMode::ABS, "LABEL_AFTER"s, u8v{0x20, 0x00}, "LABEL_AFTER"s},
        // {"alias_before", AddressMode::ABS, "ALIAS_BEFORE"s, u8v{0, 0x55}, {}}, // ZP
        {"alias_before_long", AddressMode::ABS, "ALIAS_BEFORE_LONG"s, u8v{0x34, 0x12}, {}},

        // | Relative            |          aaaa            |
        {"label_before", AddressMode::REL, "LABEL_BEFORE"s, u8v{0xee}, "LABEL_BEFORE"s},
        {"label_after", AddressMode::REL, "LABEL_AFTER"s, u8v{0x0d}, "LABEL_AFTER"s},
        {"alias_before", AddressMode::REL, "ALIAS_BEFORE"s, std::nullopt, {}},
        {"alias_before_long", AddressMode::REL, "ALIAS_BEFORE_LONG"s, std::nullopt, {}},

        // | Indirect Absolute   |          (aaaa)          |
        {"value", AddressMode::ABS_IND, "($1234)"s, u8v{0x34, 0x12}, {}},
        {"label_before", AddressMode::ABS_IND, "(LABEL_BEFORE)"s, u8v{0x01, 0x00}, "LABEL_BEFORE"s},
        {"label_after", AddressMode::ABS_IND, "(LABEL_AFTER)"s, u8v{0x20, 0x00}, "LABEL_AFTER"s},
        {"alias_before", AddressMode::ABS_IND, "(ALIAS_BEFORE)"s, std::nullopt, {}},
        {"alias_before_long", AddressMode::ABS_IND, "(ALIAS_BEFORE_LONG)"s, u8v{0x34, 0x12}, {}},

        // | Zero Page           |          aa              |
        {"value", AddressMode::ZP, "$12"s, u8v{0x12}, {}},
        // {"label_before", AddressMode::ZP, "LABEL_BEFORE"s, std::nullopt, {}}, //ABS
        // {"label_after", AddressMode::ZP, "LABEL_AFTER"s, std::nullopt, {}}, //ABS/
        {"alias_before", AddressMode::ZP, "ALIAS_BEFORE"s, u8v{0x55}, {}},
        // {"alias_before_long", AddressMode::ZP, "ALIAS_BEFORE_LONG"s, std::nullopt, {}}, //ABS

        // | Zero Page Indexed,X |          aa,X            |
        {"value", AddressMode::ZPX, "$12,X"s, u8v{0x12}, {}},
        // {"label_before", AddressMode::ZPX, "LABEL_BEFORE,X"s, std::nullopt, {}},//ABSX
        // {"label_after", AddressMode::ZPX, "LABEL_AFTER,X"s, std::nullopt, {}},//ABSX
        {"alias_before", AddressMode::ZPX, "ALIAS_BEFORE,X"s, u8v{0x55}, {}},
        // {"alias_before_long", AddressMode::ZPX, "ALIAS_BEFORE_LONG,X"s, std::nullopt, {}}, //ABSX

        // | Zero Page Indexed,Y |          aa,Y            |
        {"value", AddressMode::ZPY, "$12,Y"s, u8v{0x12}, {}},
        // {"label_before", AddressMode::ZPY, "LABEL_BEFORE,Y"s, std::nullopt, {}}, //ABSY
        // {"label_after", AddressMode::ZPY, "LABEL_AFTER,Y"s, std::nullopt, {}}, //ABSY
        {"alias_before", AddressMode::ZPY, "ALIAS_BEFORE,Y"s, u8v{0x55}, {}},
        // {"alias_before_long", AddressMode::ZPY, "ALIAS_BEFORE_LONG,Y"s, std::nullopt, {}}, //ABSY

        // | Absolute Indexed,X  |          aaaa,X          |
        {"value", AddressMode::ABSX, "$1234,X"s, u8v{0x34, 0x12}, {}},
        {"label_before", AddressMode::ABSX, "LABEL_BEFORE,X"s, u8v{0x01, 0x00}, "LABEL_BEFORE"s},
        {"label_after", AddressMode::ABSX, "LABEL_AFTER,X"s, u8v{0x20, 0x00}, "LABEL_AFTER"s},
        // {"alias_before", AddressMode::ABSX, "ALIAS_BEFORE,X"s, std::nullopt, {}}, // ZPX
        {"alias_before_long", AddressMode::ABSX, "ALIAS_BEFORE_LONG,X"s, u8v{0x34, 0x12}, {}},

        // | Absolute Indexed,Y  |          aaaa,Y          |
        {"value", AddressMode::ABSY, "$1234,Y"s, u8v{0x34, 0x12}, {}},
        {"label_before", AddressMode::ABSY, "LABEL_BEFORE,Y"s, u8v{0x01, 0x00}, "LABEL_BEFORE"s},
        {"label_after", AddressMode::ABSY, "LABEL_AFTER,Y"s, u8v{0x20, 0x00}, "LABEL_AFTER"s},
        // {"alias_before", AddressMode::ABSY, "ALIAS_BEFORE,Y"s, std::nullopt, {}}, // ZPY
        {"alias_before_long", AddressMode::ABSY, "ALIAS_BEFORE_LONG,Y"s, u8v{0x34, 0x12}, {}},

        // | Indexed Indirect    |          (aa,X)          |
        {"value", AddressMode::INDX, "($12,X)"s, u8v{0x12}, {}},
        {"label_before", AddressMode::INDX, "(LABEL_BEFORE,X)"s, std::nullopt, {}},
        {"label_after", AddressMode::INDX, "(LABEL_AFTER,X)"s, std::nullopt, {}},
        {"alias_before", AddressMode::INDX, "(ALIAS_BEFORE,X)"s, u8v{0x55}, {}},
        {"alias_before_long", AddressMode::INDX, "(ALIAS_BEFORE_LONG,X)"s, std::nullopt, {}},

        // | Indirect Indexed    |          (aa),Y          |
        {"value", AddressMode::INDY, "($12),Y"s, u8v{0x12}, {}},
        {"label_before", AddressMode::INDY, "(LABEL_BEFORE),Y"s, std::nullopt, {}},
        {"label_after", AddressMode::INDY, "(LABEL_AFTER),Y"s, std::nullopt, {}},
        {"alias_before", AddressMode::INDY, "(ALIAS_BEFORE),Y"s, u8v{0x55}, {}},
        {"alias_before_long", AddressMode::INDY, "(ALIAS_BEFORE_LONG),Y"s, std::nullopt, {}},
        // clang-format on
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
            if (variant.mode != filter) {
                continue;
            }
            auto LABEL_BEFORE = std::make_shared<SymbolInfo>(
                SymbolInfo{"LABEL_BEFORE", 1_addr, std::nullopt, false});
            auto LABEL = std::make_shared<SymbolInfo>(
                SymbolInfo{"LABEL", 0x0010_addr, std::nullopt, false});
            auto LABEL_AFTER = std::make_shared<SymbolInfo>(
                SymbolInfo{"LABEL_AFTER", 0x0020_addr, std::nullopt, false});

            auto ALIAS_BEFORE =
                std::make_shared<ValueAlias>(ValueAlias{"ALIAS_BEFORE", {0x55}});
            auto ALIAS_BEFORE_LONG = std::make_shared<ValueAlias>(
                ValueAlias{"ALIAS_BEFORE_LONG", {0x34, 0x12}});
            auto ALIAS_AFTER =
                std::make_shared<ValueAlias>(ValueAlias{"ALIAS_AFTER", {0xaa}});
            auto ALIAS_AFTER_LONG = std::make_shared<ValueAlias>(
                ValueAlias{"ALIAS_AFTER_LONG", {0x67, 0x89}});

            std::string name = fmt::format("{}_{}_{}", instruction.first,
                                           to_string(variant.mode), variant.name);
            if (name.back() == '_') {
                name.pop_back();
            }

            auto code_format = R"==(
ALIAS_BEFORE=$55
ALIAS_BEFORE_LONG=$1234

.org 0x0000
    NOP
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

ALIAS_AFTER=0xaa
ALIAS_AFTER_LONG=0x6789
)=="s;
            std::string code =
                fmt::format(code_format, instruction.first, variant.test_string);
            bool skip = false;
            if (auto skip_it = kSkipScenarios.find(variant.mode);
                skip_it != kSkipScenarios.end()) {
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
            if (opcode != instruction.second.end() && variant.test_data.has_value()) {
                auto &test_data = *variant.test_data;
                auto [opcodeinfo, is] = opcode->second;
                opcode_instruction_set = is;
                SparseBinaryCode bin_code;
                bin_code.PutBytes(0x0000, {INS_NOP, INS_NOP});
                bin_code.PutBytes(0x0010, {INS_NOP, opcodeinfo.opcode});
                bin_code.PutBytes(0x0012, test_data);
                bin_code.PutBytes(static_cast<Address_t>(0x0012 + test_data.size()),
                                  {INS_NOP});
                bin_code.PutBytes(0x0020, {INS_NOP});

                expected = Program{
                    .sparse_binary_code = bin_code,
                    .symbols =
                        {
                            {"LABEL_BEFORE", LABEL_BEFORE},
                            {"LABEL", LABEL},
                            {"LABEL_AFTER", LABEL_AFTER},
                        },
                    .aliases =
                        {
                            {"ALIAS_BEFORE", ALIAS_BEFORE},
                            {"ALIAS_BEFORE_LONG", ALIAS_BEFORE_LONG},
                            {"ALIAS_AFTER", ALIAS_AFTER},
                            {"ALIAS_AFTER_LONG", ALIAS_AFTER_LONG},
                        },
                    .relocations = {},
                };
                if (!variant.relocation.empty()) {
                    auto l = expected->symbols[variant.relocation];
                    auto ri = RelocationInfo{
                        .target_symbol = l,
                        .position = 0x0012,
                        .mode = variant.mode == AddressMode::REL
                                    ? RelocationMode::Relative
                                    : RelocationMode::Absolute,
                    };
                    auto sri = std::make_shared<RelocationInfo>(ri);
                    expected->relocations.insert(sri);
                }
            }

            test_cases.emplace_back(name, code, expected, opcode_instruction_set);
        }
    };
    return test_cases;
}

auto GetTestName() {
    return [](auto &info) { return std::get<0>(info.param); };
}

INSTANTIATE_TEST_SUITE_P(IM, CompilerTest,
                         ::testing::ValuesIn(GenTestCases(AddressMode::IM)),
                         GetTestName());
INSTANTIATE_TEST_SUITE_P(Implied, CompilerTest,
                         ::testing::ValuesIn(GenTestCases(AddressMode::Implied)),
                         GetTestName());
INSTANTIATE_TEST_SUITE_P(ABS, CompilerTest,
                         ::testing::ValuesIn(GenTestCases(AddressMode::ABS)),
                         GetTestName());
INSTANTIATE_TEST_SUITE_P(ZP, CompilerTest,
                         ::testing::ValuesIn(GenTestCases(AddressMode::ZP)),
                         GetTestName());
INSTANTIATE_TEST_SUITE_P(ZPX, CompilerTest,
                         ::testing::ValuesIn(GenTestCases(AddressMode::ZPX)),
                         GetTestName());
INSTANTIATE_TEST_SUITE_P(ZPY, CompilerTest,
                         ::testing::ValuesIn(GenTestCases(AddressMode::ZPY)),
                         GetTestName());
INSTANTIATE_TEST_SUITE_P(ABSX, CompilerTest,
                         ::testing::ValuesIn(GenTestCases(AddressMode::ABSX)),
                         GetTestName());
INSTANTIATE_TEST_SUITE_P(ABSY, CompilerTest,
                         ::testing::ValuesIn(GenTestCases(AddressMode::ABSY)),
                         GetTestName());
INSTANTIATE_TEST_SUITE_P(INDX, CompilerTest,
                         ::testing::ValuesIn(GenTestCases(AddressMode::INDX)),
                         GetTestName());
INSTANTIATE_TEST_SUITE_P(INDY, CompilerTest,
                         ::testing::ValuesIn(GenTestCases(AddressMode::INDY)),
                         GetTestName());
INSTANTIATE_TEST_SUITE_P(ACC, CompilerTest,
                         ::testing::ValuesIn(GenTestCases(AddressMode::ACC)),
                         GetTestName());
INSTANTIATE_TEST_SUITE_P(REL, CompilerTest,
                         ::testing::ValuesIn(GenTestCases(AddressMode::REL)),
                         GetTestName());
INSTANTIATE_TEST_SUITE_P(ABS_IND, CompilerTest,
                         ::testing::ValuesIn(GenTestCases(AddressMode::ABS_IND)),
                         GetTestName());

} // namespace
} // namespace emu::emu6502::test