#include <assembler/text_utils.hpp>
#include <gtest/gtest.h>
#include <optional>

using namespace std::string_view_literals;
using namespace std::string_literals;
using namespace emu6502::assembler;

using TokenizerTestArg = std::tuple<std::string, std::vector<std::string_view>>;
class TokenizerTest : public testing::Test, public ::testing::WithParamInterface<TokenizerTestArg> {
public:
};

TEST_P(TokenizerTest, ) {
    auto &[input, output] = GetParam();
    auto r = Tokenize(std::string_view(input));

    std::cout << "T:'" << input << "'\n";
    for (auto &item : output) {
        std::cout << "O:'" << item << "'\n";
    }
    for (auto &item : r) {
        std::cout << "R:'" << item << "'\n";
    }
    EXPECT_EQ(output, r);
}

INSTANTIATE_TEST_SUITE_P(, TokenizerTest,
                         ::testing::Values(TokenizerTestArg{" test test   test "s, {"test"sv, "test"sv, "test"sv}},
                                           TokenizerTestArg{" test,test,test "s, {"test"sv, "test"sv, "test"sv}},
                                           TokenizerTestArg{" test,test test "s, {"test"sv, "test"sv, "test"sv}},
                                           TokenizerTestArg{" test\ttest\ttest "s, {"test"sv, "test"sv, "test"sv}},
                                           TokenizerTestArg{" test:\ttest:\ttest"s, {"test:"sv, "test:"sv, "test"}},
                                           TokenizerTestArg{".test.test:\ttest"s, {".test.test:"sv, "test"sv}},
                                           TokenizerTestArg{"test;test"s, {"test"sv}},
                                           TokenizerTestArg{"test#test"s, {"test"sv}},
                                           TokenizerTestArg{"    \t\t\t \n\n\n   "s, {}},
                                           TokenizerTestArg{"test#test;test"s, {"test"sv}}, TokenizerTestArg{""s, {}}));

using u8v = std::vector<uint8_t>;
using ArgumentParseTestArg = std::tuple<std::string, std::optional<InstructionArgument>>;
class ArgumentParseTest : public testing::Test, public ::testing::WithParamInterface<ArgumentParseTestArg> {
public:
};

TEST_P(ArgumentParseTest, ) {
    auto &[input, output] = GetParam();

    if (output.has_value()) {
        InstructionArgument r;
        std::cout << "E: " << output.value().to_string() << "\n";
        EXPECT_NO_THROW(r = ParseInstructionArgument(std::string_view(input)));
        std::cout << "R: " << r.to_string() << "\n";
        EXPECT_EQ(output, r);
    } else {
        EXPECT_THROW(
            {
                auto r = ParseInstructionArgument(std::string_view(input));
                std::cout << "R: " << r.to_string() << "\n";
            },
            std::runtime_error);
    }
}

using AM = emu6502::cpu::opcode::AddressMode;
INSTANTIATE_TEST_SUITE_P(, ArgumentParseTest,
                         ::testing::Values(
                             // +---------------------+--------------------------+
                             // |      mode           |     assembler format     |
                             // +=====================+==========================+

                             // | Immediate           |          #aa             |
                             ArgumentParseTestArg{"#$FF"s, InstructionArgument{{AM::Immediate}, u8v{0xFF}}},  //
                             ArgumentParseTestArg{"#$FFFF"s, std::nullopt},                                   //
                             ArgumentParseTestArg{"#LABEL"s, InstructionArgument{{AM::Immediate}, "LABEL"s}}, //

                             // | Absolute            |          aaaa            |
                             ArgumentParseTestArg{"$55aa"s, InstructionArgument{{AM::ABS}, u8v{0xaa, 0x55}}}, //

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

                             // | Zero Page Indexed,Y |          aa,Y            |
                             // | Absolute Indexed,Y  |          aaaa,Y          |
                             ArgumentParseTestArg{"LABEL,Y"s, InstructionArgument{{AM::ZPY, AM::ABSY}, "LABEL"s}}, //

                             // | Zero Page           |          aa              |
                             // | Absolute            |          aaaa            |
                             // | Relative            |          aaaa            |
                             ArgumentParseTestArg{"LABEL"s,
                                                  InstructionArgument{{AM::ABS, AM::ZP, AM::REL}, "LABEL"s}}, //

                             // | Indexed Indirect    |          (aa,X)          |
                             ArgumentParseTestArg{"($FF,X)"s, InstructionArgument{{AM::INDX}, u8v{0xFF}}},  //
                             ArgumentParseTestArg{"(LABEL,X)"s, InstructionArgument{{AM::INDX}, "LABEL"s}}, //

                             // | Indirect Indexed    |          (aa),Y          |
                             ArgumentParseTestArg{"($FF),Y"s, InstructionArgument{{AM::INDY}, u8v{0xFF}}},  //
                             ArgumentParseTestArg{"(LABEL),Y"s, InstructionArgument{{AM::INDY}, "LABEL"s}}, //

                             // | Implied             |                          |
                             ArgumentParseTestArg{""s, InstructionArgument{{AM::Implied}, nullptr}}, //
                             // | Accumulator         |          A               |
                             ArgumentParseTestArg{"A"s, InstructionArgument{{AM::ACC}, nullptr}} //
                             ));
