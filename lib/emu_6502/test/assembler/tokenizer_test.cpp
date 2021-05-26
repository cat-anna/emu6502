#include "emu6502/assembler/tokenizer.hpp"
#include <gtest/gtest.h>
#include <sstream>

namespace emu::emu6502::test {
namespace {

using namespace std::string_view_literals;
using namespace std::string_literals;
using namespace emu::emu6502::assembler;

using TokenizerTestArg = std::tuple<std::string, std::vector<std::string>>;
class TokenizerTest : public testing::Test, public ::testing::WithParamInterface<TokenizerTestArg> {};

TEST_P(TokenizerTest, ) {
    auto &[input, output] = GetParam();

    std::stringstream ss(input);

    Tokenizer t{ss, "test_string"};
    std::vector<std::string> r;
    while (t.HasInput()) {
        auto l = t.NextLine();
        while (l.HasInput()) {
            if (auto tok = l.NextToken(); tok) {
                auto s = tok.String();
                r.emplace_back(s);
            }
        }
    }

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
                         ::testing::ValuesIn({
                             //basic
                             TokenizerTestArg{" test test   test  \t \t"s, {"test"s, "test"s, "test"s}}, //
                             TokenizerTestArg{"  test   #$FF  \t \t"s, {"test"s, "#$FF"s}},              //
                             TokenizerTestArg{";  test     \t \t"s, {}},                                 //
                             TokenizerTestArg{" test,test test "s, {"test"s, "test"s, "test"s}},         //
                             TokenizerTestArg{" test\ttest\ttest "s, {"test"s, "test"s, "test"s}},       //
                             TokenizerTestArg{" test:\ttest:\ttest"s, {"test:"s, "test:"s, "test"}},     //
                             TokenizerTestArg{".test.test:\ttest"s, {".test.test:"s, "test"s}},          //
                             TokenizerTestArg{"test;test"s, {"test"s}},                                  //
                             TokenizerTestArg{"test#test"s, {"test#test"s}},                             //
                             TokenizerTestArg{"    \t\t\t \t\t\t   "s, {}},                              //
                             TokenizerTestArg{""s, {}},                                                  //
                             //string and escape characters
                             TokenizerTestArg{R"=("a")="s, {"\"a\""}},                                     //
                             TokenizerTestArg{R"=(  "a b"  )="s, {"\"a b\""}},                             //
                             TokenizerTestArg{R"=("a" "b")="s, {"\"a\"", "\"b\""}},                        //
                             TokenizerTestArg{R"=("|\n|" "|\0|")="s, {"\"|\n|\"", "\"|\0|\""s}},           //
                             TokenizerTestArg{R"=("\\" "\"")="s, {"\"\\\"", "\"\"\""}},                    //
                             TokenizerTestArg{R"=("\a\b\t\n\v\f\r")="s, {"\"\a\b\t\n\v\f\r\""}},           //
                             TokenizerTestArg{R"=("\07\08\x09\xA\xB\x0C\x0D")="s, {"\"\a\b\t\n\v\f\r\""}}, //
                         }));

} // namespace
} // namespace emu::emu6502::test
