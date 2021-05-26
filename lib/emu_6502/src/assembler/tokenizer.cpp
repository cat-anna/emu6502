#include "emu6502/assembler/tokenizer.hpp"
#include <algorithm>
#include <stdexcept>

namespace emu::emu6502::assembler {

namespace {

std::tuple<char, size_t> ParseEscapeSequence(std::string_view input) {
    if (input.empty()) {
        //todo:
        throw false;
    }

    auto c = input[0];
    switch (c) {
    case 'a':
        return {'\x07', 1};
    case 'b':
        return {'\x08', 1};
    case 't':
        return {'\x09', 1};
    case 'n':
        return {'\x0a', 1};
    case 'v':
        return {'\x0b', 1};
    case 'f':
        return {'\x0c', 1};
    case 'r':
        return {'\x0d', 1};
    case '\\':
    case '"':
        // case '\'':
        return {c, 1};

    default:
        if (isdigit(c) != 0 || c == 'x') {
            size_t consumed = 0;
            int base = 10;
            if (c == 'x') {
                input.remove_prefix(1);
                ++consumed;
                base = 16;
            }

            char *end = nullptr;
            auto r = std::strtoul(input.data(), &end, base);
            if (static_cast<size_t>(end - input.data()) > input.size() || r > std::numeric_limits<uint8_t>::max()) {
                throw std::runtime_error("escape x sequence error");
            }
            consumed += end - input.data();
            return {static_cast<char>(r), consumed};
        }
        throw std::runtime_error("escape sequence error");
    }
}

std::tuple<std::string, size_t> ParseQuotedString(std::string_view input) {
    bool ended = false;
    std::string out;
    out += input[0];
    size_t consumed = 1;
    input.remove_prefix(1);

    while (!ended && !input.empty()) {
        auto c = input[0];
        ++consumed;
        input.remove_prefix(1);

        if (c == '\\') {
            auto [character, escape_consumed] = ParseEscapeSequence(input);
            consumed += escape_consumed;
            input.remove_prefix(escape_consumed);
            out += character;
            continue;
        }

        if (c == '"') {
            ended = true;
        }
        out += c;
    }

    if (!ended) {
        throw false; //TODO
    }

    return {out, consumed};
}

} // namespace

//-----------------------------------------------------------------------------

std::string Token::Upper() const {
    std::string r;
    transform(value.begin(), value.end(), std::back_inserter(r),
              [](char c) -> char { return static_cast<char>(::toupper(c)); });
    return r;
}

std::string Token::Lower() const {
    std::string r;
    transform(value.begin(), value.end(), std::back_inserter(r),
              [](char c) -> char { return static_cast<char>(::tolower(c)); });
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

    if (line.empty()) {
        //todo: throw?
        return Token{*this, Location(), std::string_view{}};
    }

    const auto location = Location();

    switch (line[0]) {
    case '"': {
        auto [token, consumed] = ParseQuotedString(line);
        column += consumed;
        line.remove_prefix(consumed);
        ConsumeUntilNextToken();
        return Token{*this, location, token};
    }
    default: {
        auto pos = line.find_first_of("\t\n ;,\"");
        if (pos == 0) {
            ++column;
            line.remove_prefix(1);
            return NextToken();
        }
        if (pos == std::string_view::npos) {
            pos = line.size();
        }
        auto token = line.substr(0, pos);
        column += pos;
        line.remove_prefix(pos);
        ConsumeUntilNextToken();
        return Token{*this, location, token};
    }
    }
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

} // namespace emu::emu6502::assembler
