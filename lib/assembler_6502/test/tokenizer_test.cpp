#include "assembler_6502/tokenizer.hpp"
#include <gtest/gtest.h>
#include <sstream>

using namespace std::string_view_literals;
using namespace std::string_literals;
using namespace emu::assembler6502;

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
                             TokenizerTestArg{" test test   test  \t \t"s, {"test"s, "test"s, "test"s}},
                             TokenizerTestArg{"  test   #$FF  \t \t"s, {"test"s, "#$FF"s}}, //
                             TokenizerTestArg{";  test     \t \t"s, {}},
                             TokenizerTestArg{" test,test test "s, {"test"s, "test"s, "test"s}},
                             TokenizerTestArg{" test\ttest\ttest "s, {"test"s, "test"s, "test"s}},
                             TokenizerTestArg{" test:\ttest:\ttest"s, {"test:"s, "test:"s, "test"}},
                             TokenizerTestArg{".test.test:\ttest"s, {".test.test:"s, "test"s}},
                             TokenizerTestArg{"test;test"s, {"test"s}}, //
                             TokenizerTestArg{"test#test"s, {"test#test"s}},
                             TokenizerTestArg{"    \t\t\t \t\t\t   "s, {}},
                             TokenizerTestArg{""s, {}},
                         }));
