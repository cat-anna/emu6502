#include "emu6502/assembler/tokenizer.hpp"
#include <algorithm>
#include <gtest/gtest.h>
#include <sstream>

namespace emu::emu6502::test {
namespace {

using namespace std::string_view_literals;
using namespace std::string_literals;
using namespace emu::emu6502::assembler;

using TokenizerTestArg = std::tuple<bool, std::string, std::vector<std::string>>;
class TokenizerTest : public testing::Test, public ::testing::WithParamInterface<TokenizerTestArg> {};

TEST_P(TokenizerTest, ) {
    auto &[expect_throw, input, output] = GetParam();

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
                             TokenizerTestArg{false, " test test   test  \t \t"s, {"test"s, "test"s, "test"s}}, //
                             TokenizerTestArg{false, "  test   #$FF  \t \t"s, {"test"s, "#$FF"s}},              //
                             TokenizerTestArg{false, ";  test     \t \t"s, {}},                                 //
                             TokenizerTestArg{false, " test,test test "s, {"test"s, ","s, "test"s, "test"s}},   //
                             TokenizerTestArg{false, " test\ttest\ttest "s, {"test"s, "test"s, "test"s}},       //
                             TokenizerTestArg{false, " test:\ttest:\ttest"s, {"test:"s, "test:"s, "test"}},     //
                             TokenizerTestArg{false, ".test.test:\ttest"s, {".test.test:"s, "test"s}},          //
                             TokenizerTestArg{false, "test;test"s, {"test"s}},                                  //
                             TokenizerTestArg{false, "test#test"s, {"test#test"s}},                             //
                             TokenizerTestArg{false, "    \t\t\t \t\t\t   "s, {}},                              //
                             TokenizerTestArg{false, ""s, {}},                                                  //
                             TokenizerTestArg{false, "test=test"s, {"test"s, "="s, "test"s}},                   //
                             TokenizerTestArg{false, " test = test "s, {"test"s, "="s, "test"s}},               //
                             TokenizerTestArg{false, " (test) = (test) "s, {"(test)"s, "="s, "(test)"s}},       //
                             //string and escape characters
                             TokenizerTestArg{false, R"=("a")="s, {"\"a\""}},                                     //
                             TokenizerTestArg{false, R"=(  "a b"  )="s, {"\"a b\""}},                             //
                             TokenizerTestArg{false, R"=("a" "b")="s, {"\"a\"", "\"b\""}},                        //
                             TokenizerTestArg{false, R"=("|\n|" "|\0|")="s, {"\"|\n|\"", "\"|\0|\""s}},           //
                             TokenizerTestArg{false, R"=("\\" "\"")="s, {"\"\\\"", "\"\"\""}},                    //
                             TokenizerTestArg{false, R"=("\a\b\t\n\v\f\r")="s, {"\"\a\b\t\n\v\f\r\""}},           //
                             TokenizerTestArg{false, R"=("\07\08\x09\xA\xB\x0C\x0D")="s, {"\"\a\b\t\n\v\f\r\""}}, //
                         }));

class TokenizerListTest : public testing::Test, public ::testing::WithParamInterface<TokenizerTestArg> {};

TEST_P(TokenizerListTest, ) {
    auto &[expect_throw, input, output] = GetParam();

    std::stringstream ss(input);

    Tokenizer t{ss, "test_string"};
    std::vector<std::string> r;
    auto line = t.NextLine();
    auto list = line.TokenList(",");

    std::vector<Token> list_tokens;
    auto func = [&] { list_tokens = list.Vector(); };
    if (expect_throw) {
        EXPECT_THROW(func(), TokenizerException);
    } else {
        EXPECT_NO_THROW(func());
    }

    std::transform(list_tokens.begin(), list_tokens.end(), std::back_inserter(r),
                   [](auto &item) { return item.String(); });
    std::cout << "T:'" << input << "'\n";
    for (auto &item : r) {
        std::cout << "R:'" << item << "'\n";
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

INSTANTIATE_TEST_SUITE_P(, TokenizerListTest,
                         ::testing::ValuesIn({
                             TokenizerTestArg{false, "a,b,c"s, {"a"s, "b"s, "c"s}},       //
                             TokenizerTestArg{false, "a,b,c,"s, {"a"s, "b"s, "c"s}},      //
                             TokenizerTestArg{false, " a , b , c "s, {"a"s, "b"s, "c"s}}, //
                             TokenizerTestArg{false, "a"s, {"a"s}},                       //
                             TokenizerTestArg{false, "a,"s, {"a"s}},                      //
                             TokenizerTestArg{true, ",,b,c"s, {}},                        //
                             TokenizerTestArg{true, ",,"s, {}},                           //
                             TokenizerTestArg{true, ","s, {}},                            //
                             TokenizerTestArg{true, "a,,"s, {}},                          //
                         }));

} // namespace
} // namespace emu::emu6502::test
