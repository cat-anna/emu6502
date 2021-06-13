#include "assembler/instruction_argument.hpp"
#include "emu_6502/assembler/compilation_error.hpp"
#include "emu_core/byte_utils.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <optional>

namespace emu::emu6502::test {
namespace {

using namespace std::string_view_literals;
using namespace std::string_literals;
using namespace emu::emu6502::assembler;

using u8v = std::vector<uint8_t>;

const AliasMap kTestAliases = {
    {"byte", std::make_shared<ValueAlias>(ValueAlias{"byte", {1}})},
    {"word", std::make_shared<ValueAlias>(ValueAlias{"word", {1, 2}})},
};

const LabelMap kTestLabelMap = {
    {"L1", std::make_shared<LabelInfo>(LabelInfo{"L1", 1_u8, false, {}})},
    {"L2", std::make_shared<LabelInfo>(LabelInfo{"L2", 2_u8, false, {}})},
};

using ArgumentParseTestArg = std::tuple<std::string, std::optional<InstructionArgument>>;
class ArgumentParseTest : public testing::Test, public ::testing::WithParamInterface<ArgumentParseTestArg> {};

TEST_P(ArgumentParseTest, ) {
    auto &[input, output] = GetParam();

    Token test_token{nullptr, {}, input};
    if (output.has_value()) {
        InstructionArgument r;
        std::cout << "E: " << to_string(output.value()) << "\n";
        EXPECT_NO_THROW(r = ParseInstructionArgument(test_token, kTestAliases));
        std::cout << "R: " << to_string(r) << "\n";
        EXPECT_EQ(output, r);
    } else {
        EXPECT_THROW(
            {
                try {
                    auto r = ParseInstructionArgument(test_token, kTestAliases);
                    std::cout << "R: " << to_string(r) << "\n";
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
} // namespace

std::vector<ArgumentParseTestArg> GetTestCases() {
    using AM = AddressMode;
    return {
        // +---------------------+--------------------------+
        // |      mode           |     assembler format     |
        // +=====================+==========================+

        // | Immediate           |          #aa             |
        ArgumentParseTestArg{"#$FF"s, InstructionArgument{{AM::Immediate}, u8v{0xFF}}},  //
        ArgumentParseTestArg{"#$FFFF"s, std::nullopt},                                   //
        ArgumentParseTestArg{"#LABEL"s, InstructionArgument{{AM::Immediate}, "LABEL"s}}, //
        ArgumentParseTestArg{"#byte"s, InstructionArgument{{AM::Immediate}, u8v{1}}},    //
        ArgumentParseTestArg{"#word"s, std::nullopt},                                    //
        // | Indirect Absolute   |          (aaaa)          |
        ArgumentParseTestArg{"(LABEL)"s, InstructionArgument{{AM::ABS_IND}, "LABEL"s}},        //
        ArgumentParseTestArg{"($55aa)"s, InstructionArgument{{AM::ABS_IND}, u8v{0xaa, 0x55}}}, //
        ArgumentParseTestArg{"($55)"s, std::nullopt},                                          //

        // | Absolute Indexed,X  |          aaaa,X          |
        ArgumentParseTestArg{"$55aa,X"s, InstructionArgument{{AM::ABSX}, u8v{0xaa, 0x55}}}, //

        // | Absolute Indexed,Y  |          aaaa,Y          |
        ArgumentParseTestArg{"$55aa,Y"s, InstructionArgument{{AM::ABSY}, u8v{0xaa, 0x55}}}, //

        // | Zero Page Indexed,X |          aa,X            |
        ArgumentParseTestArg{"$55,X"s, InstructionArgument{{AM::ZPX}, u8v{0x55}}}, //

        // | Zero Page Indexed,Y |          aa,Y            |
        ArgumentParseTestArg{"$55,Y"s, InstructionArgument{{AM::ZPY}, u8v{0x55}}}, //

        // | Zero Page Indexed,X |          aa,X            |
        // | Absolute Indexed,X  |          aaaa,X          |
        ArgumentParseTestArg{"LABEL,X"s, InstructionArgument{{AM::ZPX, AM::ABSX}, "LABEL"s}}, //
        ArgumentParseTestArg{"byte,X"s, InstructionArgument{{AM::ZPX}, u8v{1}}},              //
        ArgumentParseTestArg{"word,X"s, InstructionArgument{{AM::ABSX}, u8v{1, 2}}},          //

        // | Zero Page Indexed,Y |          aa,Y            |
        // | Absolute Indexed,Y  |          aaaa,Y          |
        ArgumentParseTestArg{"LABEL,Y"s, InstructionArgument{{AM::ZPY, AM::ABSY}, "LABEL"s}}, //
        ArgumentParseTestArg{"byte,Y"s, InstructionArgument{{AM::ZPY}, u8v{1}}},              //
        ArgumentParseTestArg{"word,Y"s, InstructionArgument{{AM::ABSY}, u8v{1, 2}}},          //

        // | Zero Page           |          aa              |
        // | Absolute            |          aaaa            |
        // | Relative            |          aaaa            |
        ArgumentParseTestArg{"LABEL"s, InstructionArgument{{AM::ABS, AM::ZP, AM::REL}, "LABEL"s}}, //
        ArgumentParseTestArg{"$12"s, InstructionArgument{{AM::ZP}, u8v{0x12}}},                    //
        ArgumentParseTestArg{"$55aa"s, InstructionArgument{{AM::ABS}, u8v{0xaa, 0x55}}},           //
        ArgumentParseTestArg{"byte"s, InstructionArgument{{AM::ZP}, u8v{1}}},                      //
        ArgumentParseTestArg{"word"s, InstructionArgument{{AM::ABS}, u8v{1, 2}}},                  //

        // | Indexed Indirect    |          (aa,X)          |
        ArgumentParseTestArg{"($FF,X)"s, InstructionArgument{{AM::INDX}, u8v{0xFF}}},  //
        ArgumentParseTestArg{"(LABEL,X)"s, InstructionArgument{{AM::INDX}, "LABEL"s}}, //
        ArgumentParseTestArg{"(byte,X)"s, InstructionArgument{{AM::INDX}, u8v{1}}},    //
        ArgumentParseTestArg{"(word,X)"s, std::nullopt},                               //

        // | Indirect Indexed    |          (aa),Y          |
        ArgumentParseTestArg{"($FF),Y"s, InstructionArgument{{AM::INDY}, u8v{0xFF}}},  //
        ArgumentParseTestArg{"(LABEL),Y"s, InstructionArgument{{AM::INDY}, "LABEL"s}}, //
        ArgumentParseTestArg{"(byte),Y"s, InstructionArgument{{AM::INDY}, u8v{1}}},    //
        ArgumentParseTestArg{"(word,Y)"s, std::nullopt},                               //

        // | Implied             |                          |
        ArgumentParseTestArg{""s, InstructionArgument{{AM::Implied}, nullptr}}, //
        // | Accumulator         |          A               |
        ArgumentParseTestArg{"A"s, InstructionArgument{{AM::ACC}, nullptr}}, //
    };
}

INSTANTIATE_TEST_SUITE_P(, ArgumentParseTest, ::testing::ValuesIn(GetTestCases()));

using GetTokenTypeTestArg = std::tuple<std::string, TokenType>;
class GetTokenTypeTest : public testing::Test, public ::testing::WithParamInterface<GetTokenTypeTestArg> {};

TEST_P(GetTokenTypeTest, ) {
    auto [input, output] = GetParam();
    Token tok{nullptr, {}, input};
    EXPECT_EQ(GetTokenType(tok, &kTestAliases, &kTestLabelMap), output);
}

std::vector<GetTokenTypeTestArg> GetTokenTypeTestCases() {
    return {
        {"unknown"s, TokenType::kUnknown}, //
        {"0x55"s, TokenType::kValue},      //
        {"0XAA55"s, TokenType::kValue},    //
        {"$AA55"s, TokenType::kValue},     //
        {"$AA55"s, TokenType::kValue},     //
        {"byte"s, TokenType::kAlias},      //
        {"word"s, TokenType::kAlias},      //
        {"L1"s, TokenType::kLabel},        //
        {"L2"s, TokenType::kLabel},        //
        {"1595"s, TokenType::kValue},      //
    };
}

INSTANTIATE_TEST_SUITE_P(, GetTokenTypeTest, ::testing::ValuesIn(GetTokenTypeTestCases()));

} // namespace
} // namespace emu::emu6502::test
