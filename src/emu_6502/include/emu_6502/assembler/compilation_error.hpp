#pragma once

#include "emu_6502/assembler/tokenizer.hpp"
#include <fmt/format.h>
#include <stdexcept>
#include <string>

namespace emu::emu6502::assembler {

enum class CompilationError {
    NoError = 0,

    Unknown,
    UnknownCommand,

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

class CompilationException : public std::exception {
public:
    CompilationException(std::string _message, CompilationError _error, Token _token = {})
        : message(std::move(_message)), error(_error), token(std::move(_token)) {}

    virtual std::string Message() const;
    const TokenLocation &Location() const { return token.location; }
    CompilationError Error() const { return error; }
    const char *what() const noexcept override { return message.c_str(); }

    template <typename... ARGS>
    static std::string FormatMessage(CompilationError err, const Token &t,
                                     const char *fmt = nullptr, ARGS &&...args) {
        if (fmt != nullptr) {
            return to_string(err) + ": " + fmt::format(fmt, std::forward<ARGS>(args)...);
        } else {
            return to_string(err) + ": " + GetDefaultMessage(err, t);
        }
    }

private:
    const std::string message;
    const CompilationError error;
    const Token token;
};

template <typename... ARGS>
[[noreturn]] inline void ThrowCompilationError(CompilationError err, Token t,
                                               const char *fmt = nullptr,
                                               ARGS &&...args) {
    auto msg =
        CompilationException::FormatMessage(err, t, fmt, std::forward<ARGS>(args)...);
    throw CompilationException(msg, err, std::move(t));
}

} // namespace emu::emu6502::assembler