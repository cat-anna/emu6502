#pragma once

#include "emu_6502/assembler/tokenizer.hpp"
#include <fmt/format.h>
#include <stdexcept>
#include <string>

namespace emu::emu6502::assembler {

enum class CompilationError {
    NoError = 0,

    Unknown,
    InternalError,
    UnknownCommand,

    InvalidEscapeSequence,
    UnfinishedQuotedString,

    UnexpectedInput,
    UnexpectedEndOfInput,
    InvalidToken,

    SymbolRedefinition,
    AliasRedefinition,

    UnknownIsr,
    InvalidIsrArgument,

    SymbolIsNotAllowed,
    AliasIsNotAllowed,

    InvalidOperandSize,
    InvalidOperandArgument,
    OperandModeNotSupported,

    InvalidCommandArgument,

};

std::string to_string(CompilationError error);
std::string GetDefaultMessage(CompilationError error, const Token &t);

class CompilationSubException : public std::exception {
public:
    CompilationSubException(std::string message, CompilationError error,
                            size_t _offset = 0)
        : message(std::move(message)), error(error), offset(_offset) {}

    CompilationSubException(const CompilationSubException &sub_exception, size_t _offset)
        : message(sub_exception.message), error(sub_exception.error),
          offset(_offset + sub_exception.offset) {}

    const char *what() const noexcept override { return message.c_str(); }

    std::string message;
    const CompilationError error;
    const size_t offset;
};

class CompilationException : public std::exception {
public:
    CompilationException(std::string _message, CompilationError _error,
                         std::optional<Token> _token = {})
        : message(std::move(_message)), error(_error), token(std::move(_token)) {}

    CompilationException(const CompilationSubException &sub_exception, Token token = {})
        : message(sub_exception.message), error(sub_exception.error),
          token(Token{std::move(token), sub_exception.offset}) {}

    virtual std::string Message() const;
    bool HasToken() const { return token.has_value(); }
    const TokenLocation &Location() const { return token->location; }
    CompilationError Error() const { return error; }
    const char *what() const noexcept override { return message.c_str(); }

    template <typename... ARGS>
    static std::string FormatMessage(CompilationError err, const std::optional<Token> &t,
                                     const char *fmt = nullptr, ARGS &&...args) {
        if (fmt != nullptr) {
            return to_string(err) + ": " + fmt::format(fmt, std::forward<ARGS>(args)...);
        } else {
            return to_string(err) + ": " + GetDefaultMessage(err, t.value_or(Token{}));
        }
    }

private:
    const std::string message;
    const CompilationError error;
    const std::optional<Token> token;
};

template <typename... ARGS>
[[noreturn]] inline void
ThrowCompilationError(CompilationError err, std::optional<Token> t,
                      const char *fmt = nullptr, ARGS &&...args) {
    auto msg =
        CompilationException::FormatMessage(err, t, fmt, std::forward<ARGS>(args)...);
    throw CompilationException(std::move(msg), err, std::move(t));
}

} // namespace emu::emu6502::assembler