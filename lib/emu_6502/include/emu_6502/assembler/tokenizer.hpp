#pragma once

#include <iostream>
#include <optional>
#include <stack>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace emu::emu6502::assembler {

struct Tokenizer;
struct LineTokenizer;
struct TokenListIterator;

struct TokenLocation {
    TokenLocation() = default;
    TokenLocation(const TokenLocation &, size_t) {}
};

std::string to_string(const TokenLocation &location);

enum class TokenizerError {
    Unknown,
    InvalidEscapeSequence,
};
std::string to_string(TokenizerError error);

class TokenizerSubException : public std::exception {
public:
    TokenizerSubException(std::string message, TokenizerError error, size_t _offset = 0)
        : message(std::move(message)), error(error), offset(_offset) {}

    TokenizerSubException(const TokenizerSubException &sub_exception, size_t _offset)
        : message(sub_exception.message), error(sub_exception.error), offset(_offset + sub_exception.offset) {}

    const char *what() const noexcept override { return message.c_str(); }

    std::string message;
    const TokenizerError error;
    const size_t offset;
};

class TokenizerException : public std::exception {
public:
    TokenizerException(std::string message, TokenizerError error, TokenLocation location = {})
        : message(std::move(message)), error(error), location(std::move(location)) {}

    TokenizerException(const TokenizerSubException &sub_exception, TokenLocation location = {})
        : message(sub_exception.message), error(sub_exception.error),
          location(std::move(location), sub_exception.offset) {}

    virtual std::string Message() const;
    const TokenLocation &Location() const { return location; }
    TokenizerError Error() const { return error; }
    virtual const char *what() const { return message.c_str(); }

private:
    const std::string message;
    const TokenizerError error;
    const TokenLocation location;
};

struct Token {
    LineTokenizer *line = nullptr;
    std::string value;
    TokenLocation location = {};

    Token() = default;
    Token(const Token &) = default;
    Token(Token &&) = default;

    Token(LineTokenizer *line, TokenLocation location, std::string input)
        : line(line), location(location), value(input) {}
    Token(LineTokenizer *line, TokenLocation location, std::string_view input)
        : line(line), value(input), location(location) {}

    operator bool() const { return !value.empty(); }
    bool operator==(std::string_view s) const { return value == s; }

    Token &operator=(const Token &) = default;
    Token &operator=(Token &&) = default;

    std::string Upper() const;
    std::string Lower() const;
    std::string_view View() const { return std::string_view(value); }
    const std::string &String() const { return value; }
};

std::string to_string(const Token &token);

struct LineTokenizer {
    LineTokenizer(Tokenizer &_tokenizer, size_t _line_number, std::string _line_storage)
        : tokenizer(_tokenizer), line_number(_line_number), line_storage(_line_storage), line(line_storage) {}

    bool HasInput();
    Token NextToken();
    TokenLocation Location() const;

    TokenListIterator TokenList(std::string separator);

private:
    Tokenizer &tokenizer;
    const size_t line_number;
    const std::string line_storage;
    size_t column = 0;
    std::string_view line;

    void ConsumeUntilNextToken();
};

struct Tokenizer {
    Tokenizer(std::istream &_input, std::string _input_name) : input(_input), input_name(_input_name) {}

    LineTokenizer NextLine();
    bool HasInput();
    TokenLocation Location() const;

private:
    std::istream &input;
    const std::string input_name;
    size_t line = 0;
};

struct TokenListIterator {
    TokenListIterator(LineTokenizer &tokenizer, std::string separator) : tokenizer(tokenizer), separator(separator) {}

    struct Iterator {
        using iterator_category = std::input_iterator_tag;
        using value_type = Token;
        using difference_type = std::ptrdiff_t;
        using pointer = Token *;
        using reference = Token &;

        LineTokenizer *parent = nullptr;
        std::string separator = {};
        std::optional<Token> current_token = std::nullopt;

        Iterator() = default;
        Iterator(LineTokenizer *parent, std::string separator) : parent(parent), separator(separator) { operator++(); }
        Token operator*();
        void operator++();
        bool operator!=(const Iterator &other);
    };

    Iterator begin() { return Iterator{&tokenizer, separator}; }
    Iterator end() { return Iterator{}; }

    std::vector<Token> Vector() { return {begin(), end()}; }

private:
    LineTokenizer &tokenizer;
    const std::string separator;
};

} // namespace emu::emu6502::assembler
