#pragma once

#include <iostream>
#include <memory>
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
    TokenLocation(TokenLocation &&) = default;
    TokenLocation(const TokenLocation &) = default;

    TokenLocation(std::shared_ptr<const std::string> input_name,
                  std::shared_ptr<const std::string> line_content, size_t line,
                  size_t column)
        : input_name(std::move(input_name)), line_content(std::move(line_content)),
          line(line), column(column) {}

    TokenLocation &operator=(const TokenLocation &) = default;
    TokenLocation &operator=(TokenLocation &&) = default;

    std::shared_ptr<const std::string> input_name = {};
    std::shared_ptr<const std::string> line_content = {};
    size_t line = 0;
    size_t column = 0;

    std::string GetDescription() const;
};

std::string to_string(const TokenLocation &location);

struct Token {
    std::string value;
    TokenLocation location = {};

    Token() = default;
    Token(const Token &) = default;
    Token(Token &&) = default;

    Token(const Token &other, size_t column_offset) : Token(other) {
        location.column += column_offset;
    }
    Token(Token &&other, size_t column_offset) : Token(other) {
        location.column += column_offset;
    }
    Token(TokenLocation location, std::string input = {})
        : value(input), location(location) {}
    Token(TokenLocation location, std::string_view input)
        : value(input), location(location) {}

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
    LineTokenizer(Tokenizer &_tokenizer, size_t _line_number,
                  std::shared_ptr<const std::string> _line_storage)
        : tokenizer(_tokenizer), line_number(_line_number), line_storage(_line_storage),
          line(*line_storage) {}

    bool HasInput();
    Token NextToken();
    TokenLocation Location() const;

    TokenListIterator TokenList(std::string separator);

private:
    Tokenizer &tokenizer;
    const size_t line_number;
    std::shared_ptr<const std::string> line_storage;
    size_t column = 0;
    std::string_view line;

    void ConsumeUntilNextToken();
};

struct Tokenizer {
    Tokenizer(std::istream &_input, std::string _input_name)
        : input(_input), input_name(std::make_shared<std::string>(_input_name)) {}

    LineTokenizer NextLine();
    bool HasInput();
    TokenLocation Location() const;

    auto GetInputName() const { return input_name; }

private:
    std::istream &input;
    const std::shared_ptr<const std::string> input_name;
    std::shared_ptr<const std::string> current_line = std::make_shared<std::string>();
    size_t line = 0;
};

struct TokenListIterator {
    TokenListIterator(LineTokenizer &tokenizer, std::string separator)
        : tokenizer(tokenizer), separator(separator) {}

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
        Iterator(LineTokenizer *parent, std::string separator)
            : parent(parent), separator(separator) {
            operator++();
        }
        Token operator*();
        void operator++();
        bool operator!=(const Iterator &other) const;
    };

    Iterator begin() { return Iterator{&tokenizer, separator}; }
    Iterator end() { return Iterator{}; }

    std::vector<Token> Vector() { return {begin(), end()}; }

private:
    LineTokenizer &tokenizer;
    const std::string separator;
};

} // namespace emu::emu6502::assembler
