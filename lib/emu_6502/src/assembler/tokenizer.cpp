#include "emu6502/assembler/tokenizer.hpp"
#include <algorithm>
#include <fmt/format.h>
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
                throw TokenizerException("Value of escape sequence exceeds uint8",
                                         TokenizerError::InvalidEscapeSequence);
            }
            consumed += end - input.data();
            return {static_cast<char>(r), consumed};
        }
        throw TokenizerException("Malformed escape sequence", TokenizerError::InvalidEscapeSequence);
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
            try {
                auto [character, escape_consumed] = ParseEscapeSequence(input);
                consumed += escape_consumed;
                input.remove_prefix(escape_consumed);
                out += character;
                continue;
            } catch (const TokenizerSubException &e) {
                throw TokenizerSubException(e, consumed);
            }
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

std::string to_string(const TokenLocation &location) {
    return "location(TODO)";
}

//-----------------------------------------------------------------------------

std::string to_string(TokenizerError error) {
    switch (error) {
    case TokenizerError::Unknown:
        return "Unknown";
    case TokenizerError::InvalidEscapeSequence:
        return "InvalidEscapeSequence";
    }
    return fmt::format("Invalid error id {}", static_cast<int>(error));
}

//-----------------------------------------------------------------------------

std::string TokenizerException::Message() const {
    return fmt::format("{} : {} : {}", to_string(location), to_string(error), what());
}

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
    return "Token{TODO}";
}

//-----------------------------------------------------------------------------

Token TokenListIterator::Iterator::operator*() {
    if (current_token.has_value()) {
        return *current_token;
    }
    throw TokenizerException("Attempt to dereference empty TokenListIterator::Iterator", TokenizerError::Unknown, {});
}

void TokenListIterator::Iterator::operator++() {
    if (parent == nullptr) {
        return;
    }

    if (!parent->HasInput()) {
        parent = nullptr;
        current_token = std::nullopt;
        return;
    }

    if (current_token.has_value()) {
        current_token = std::nullopt;
        auto next = parent->NextToken();
        if (next.value != separator) {
            throw TokenizerException("Not a separator", TokenizerError::Unknown, next.location);
        }
        operator++();
    } else {
        current_token = parent->NextToken();
        if (current_token->value == separator) {
            throw TokenizerException("no element in list", TokenizerError::Unknown, current_token->location);
        }
    }
}

bool TokenListIterator::Iterator::operator!=(const Iterator &other) {
    return parent != other.parent;
}

//-----------------------------------------------------------------------------

TokenListIterator LineTokenizer::TokenList(std::string separator) {
    return TokenListIterator(*this, separator);
}

bool LineTokenizer::HasInput() {
    ConsumeUntilNextToken();
    return !line.empty();
}

Token LineTokenizer::NextToken() {
    ConsumeUntilNextToken();

    if (line.empty()) {
        //todo: throw?
        return Token{this, Location(), std::string_view{}};
    }

    const auto location = Location();

    auto Consume = [&](size_t c) {
        if (line.size() < c) {
            throw TokenizerException("TODO ERROR", TokenizerError::Unknown, location);
        }
        auto token = line.substr(0, c);
        column += c;
        line.remove_prefix(c);
        return Token{this, location, token};
    };

    try {
        switch (line[0]) {
        case '"': {
            auto [token, consumed] = ParseQuotedString(line);
            column += consumed;
            line.remove_prefix(consumed);
            ConsumeUntilNextToken();
            return Token{this, location, token};
        }
        case ',':
        case '=':
            return Consume(1);

        default: {
            auto pos = line.find_first_of("\t\n ;,=");
            if (pos == 0) {
                throw TokenizerException("TODO ERROR 2", TokenizerError::Unknown, location);
            }
            if (pos == std::string_view::npos) {
                pos = line.size();
            }
            auto t = Consume(pos);
            ConsumeUntilNextToken();
            return t;
        }
        }
    } catch (const TokenizerSubException &e) {
        throw TokenizerException(e, location);
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
        throw TokenizerException("No more input TODO", TokenizerError::Unknown, Location());
    }

    std::string raw_line;
    std::getline(input, raw_line);
    ++line;

    return LineTokenizer(*this, line, std::move(raw_line));
}

TokenLocation Tokenizer::Location() const {
    return {};
}

} // namespace emu::emu6502::assembler
