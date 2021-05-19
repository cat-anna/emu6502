#include "assembler_6502/tokenizer.hpp"
#include <algorithm>
#include <stdexcept>

namespace emu::assembler6502 {

std::string Token::Upper() const {
    std::string r;
    transform(value.begin(), value.end(), std::back_inserter(r), ::toupper);
    return r;
}

std::string Token::Lower() const {
    std::string r;
    transform(value.begin(), value.end(), std::back_inserter(r), ::tolower);
    return r;
}

std::string to_string(const Token &token) {
    return "";
}

//-----------------------------------------------------------------------------

bool LineTokenizer::HasInput() {
    ConsumeUntilNextToken();
    return !line.empty();
}

Token LineTokenizer::NextToken() {

    ConsumeUntilNextToken();

    auto pos = line.find_first_of("\t\n ;,");
    if (pos == std::string_view::npos) {
        pos = line.size();
    }

    auto token = line.substr(0, pos);
    Token t{*this, token, Location()};
    column += pos;
    line.remove_prefix(pos);
    if (!line.empty() && line[0] != ';') {
        line.remove_prefix(1);
        ++column;
    }

    ConsumeUntilNextToken();
    return t;
}

TokenLocation LineTokenizer::Location() const {
    return {};
}

void LineTokenizer::ConsumeUntilNextToken() {
    while (!line.empty()) {
        if (::isspace(line[0]) > 0) {
            ++column;
            line.remove_prefix(1);
            continue;
        }

        switch (line[0]) {
        case ';':
            column += line.size();
            line = {};
            return;

        default:
            return;
        }
    }
}

//-----------------------------------------------------------------------------

bool Tokenizer::HasInput() {
    return !input.eof();
}

LineTokenizer Tokenizer::NextLine() {
    if (!HasInput()) {
        throw std::runtime_error("No more input TODO");
    }

    std::string raw_line;
    std::getline(input, raw_line);
    ++line;

    return LineTokenizer(*this, line, std::move(raw_line));
}

//

} // namespace emu::assembler6502
