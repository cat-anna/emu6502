#include "emu_6502/assembler/tokenizer.hpp"
#include "emu_6502/assembler/compilation_error.hpp"
#include <algorithm>
#include <fmt/format.h>
#include <stdexcept>

namespace emu::emu6502::assembler {

namespace {

std::tuple<char, size_t> ParseEscapeSequence(std::string_view input) {
    if (input.empty()) {
        throw std::runtime_error("Empty input for ParseEscapeSequence");
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
            if (static_cast<size_t>(end - input.data()) > input.size() ||
                r > std::numeric_limits<uint8_t>::max()) {
                throw CompilationException("Value of escape sequence exceeds uint8",
                                           CompilationError::InvalidEscapeSequence);
            }
            consumed += end - input.data();
            return {static_cast<char>(r), consumed};
        }
        throw CompilationException("Malformed escape sequence",
                                   CompilationError::InvalidEscapeSequence);
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
            } catch (const CompilationSubException &e) {
                throw CompilationSubException(e, consumed);
            }
        }

        if (c == '"') {
            ended = true;
        }
        out += c;
    }

    if (!ended) {
        throw CompilationSubException("Quoted string not terminated",
                                      CompilationError::UnfinishedQuotedString, consumed);
    }

    return {out, consumed};
}

} // namespace

//-----------------------------------------------------------------------------

std::string TokenLocation::GetDescription() const {
    std::string s;
    s += fmt::format("{:04}: {}\n", line, *line_content);
    s += fmt::format("{}^\n", std::string(column + 6, ' '));
    return s;
}

std::string to_string(const TokenLocation &location) {
    return fmt::format("{}:{}:{}", *location.input_name, location.line, location.column);
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
    return fmt::format("Token '{}' at {}", token.value, to_string(token.location));
}

//-----------------------------------------------------------------------------

Token TokenListIterator::Iterator::operator*() {
    if (current_token.has_value()) {
        return *current_token;
    }
    throw CompilationException("Attempt to dereference empty TokenListIterator::Iterator",
                               CompilationError::InternalError, {});
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
            throw CompilationException("Not a separator", CompilationError::Unknown,
                                       next);
        }
        operator++();
    } else {
        current_token = parent->NextToken();
        if (current_token->value == separator) {
            throw CompilationException("no element in list", CompilationError::Unknown,
                                       *current_token);
        }
    }
}

bool TokenListIterator::Iterator::operator!=(const Iterator &other) const {
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
        return Token{Location(), std::string_view{}};
    }

    auto Consume = [&](size_t c) {
        if (line.size() < c) {
            throw CompilationException("TODO ERROR", CompilationError::Unknown,
                                       Token{Location()});
        }
        auto token = line.substr(0, c);
        column += c;
        line.remove_prefix(c);
        return Token{Location(), token};
    };

    try {
        switch (line[0]) {
        case '"': {
            // Consume(2);
            // location = Location();
            auto [token, consumed] = ParseQuotedString(line);
            column += consumed;
            line.remove_prefix(consumed);
            ConsumeUntilNextToken();
            return Token{Location(), token};
        }
        case ',':
        case '=':
            return Consume(1);

        default: {
            auto pos = line.find_first_of("\t\n ;,=");
            if (pos == 0) {
                throw CompilationException("TODO ERROR 2", CompilationError::Unknown,
                                           Token{Location()});
            }
            if (pos == std::string_view::npos) {
                pos = line.size();
            }
            auto t = Consume(pos);
            ConsumeUntilNextToken();
            return t;
        }
        }
    } catch (const CompilationSubException &e) {
        throw CompilationException(e, Token{Location()});
    }
}

TokenLocation LineTokenizer::Location() const {
    return TokenLocation{
        tokenizer.GetInputName(),
        line_storage,
        line_number,
        column,
    };
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
        throw CompilationException("No more input TODO",
                                   CompilationError::UnexpectedEndOfInput,
                                   Token{Location()});
    }

    std::string raw_line;
    std::getline(input, raw_line);

    current_line = std::make_shared<std::string>(std::move(raw_line));
    ++line;

    return LineTokenizer(*this, line, current_line);
}

TokenLocation Tokenizer::Location() const {
    return TokenLocation{
        GetInputName(),
        current_line,
        line,
        current_line->size(),
    };
}

} // namespace emu::emu6502::assembler
