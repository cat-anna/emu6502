#pragma once

#include <iostream>
#include <stack>
#include <string>
#include <string_view>

namespace emu::emu6502::assembler {

struct Tokenizer;
struct LineTokenizer;

struct TokenLocation {};

struct Token {
    LineTokenizer &line;
    std::string_view value;
    TokenLocation location;

    operator bool() const { return !value.empty(); }

    std::string Upper() const;
    std::string Lower() const;
    auto String() const { return std::string(value); }
};

std::string to_string(const Token &token);

struct LineTokenizer {
public:
    LineTokenizer(Tokenizer &_tokenizer, size_t _line_number, std::string _line_storage)
        : tokenizer(_tokenizer), line_number(_line_number), line_storage(_line_storage), line(line_storage) {}

    bool HasInput();
    Token NextToken();
    TokenLocation Location() const;

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

private:
    std::istream &input;
    const std::string input_name;
    size_t line = 0;
};

} // namespace emu::emu6502::assembler
