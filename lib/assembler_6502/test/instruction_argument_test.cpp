#include "instruction_argument.hpp"
#include <gtest/gtest.h>
#include <optional>

using namespace std::string_view_literals;
using namespace std::string_literals;
using namespace emu::assembler6502;

using u8v = std::vector<uint8_t>;
using ArgumentParseTestArg = std::tuple<std::string, std::optional<InstructionArgument>>;
class ArgumentParseTest : public testing::Test, public ::testing::WithParamInterface<ArgumentParseTestArg> {
public:
};

TEST_P(ArgumentParseTest, ) {
    auto &[input, output] = GetParam();

    if (output.has_value()) {
        InstructionArgument r;
        std::cout << "E: " << to_string(output.value()) << "\n";
        EXPECT_NO_THROW(r = ParseInstructionArgument(std::string_view(input)));
        std::cout << "R: " << to_string(r) << "\n";
        EXPECT_EQ(output, r);
    } else {
        EXPECT_THROW(
            {
                auto r = ParseInstructionArgument(std::string_view(input));
                std::cout << "R: " << to_string(r) << "\n";
            },
            std::runtime_error);
    }
}

using AM = emu::cpu6502::AddressMode;
INSTANTIATE_TEST_SUITE_P(, ArgumentParseTest,
                         ::testing::ValuesIn({
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
                             ArgumentParseTestArg{"$12"s, InstructionArgument{{AM::ZP}, u8v{0x12}}},          //

                             // | Indexed Indirect    |          (aa,X)          |
                             ArgumentParseTestArg{"($FF,X)"s, InstructionArgument{{AM::INDX}, u8v{0xFF}}},  //
                             ArgumentParseTestArg{"(LABEL,X)"s, InstructionArgument{{AM::INDX}, "LABEL"s}}, //

                             // | Indirect Indexed    |          (aa),Y          |
                             ArgumentParseTestArg{"($FF),Y"s, InstructionArgument{{AM::INDY}, u8v{0xFF}}},  //
                             ArgumentParseTestArg{"(LABEL),Y"s, InstructionArgument{{AM::INDY}, "LABEL"s}}, //

                             // | Implied             |                          |
                             ArgumentParseTestArg{""s, InstructionArgument{{AM::Implied}, nullptr}}, //
                             // | Accumulator         |          A               |
                             ArgumentParseTestArg{"A"s, InstructionArgument{{AM::ACC}, nullptr}}, //
                         }));
