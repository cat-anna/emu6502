#include "emu_6502/assembler/compilation_error.hpp"

namespace emu::emu6502::assembler {

std::string to_string(CompilationError error) {
    switch (error) {
    case CompilationError::NoError:
        return "NoError";
    case CompilationError::Unknown:
        return "Unknown";
    case CompilationError::InternalError:
        return "InternalError";
    case CompilationError::InvalidEscapeSequence:
        return "InvalidEscapeSequence";
    case CompilationError::UnfinishedQuotedString:
        return "UnfinishedQuotedString";
    case CompilationError::UnexpectedInput:
        return "UnexpectedInput";
    case CompilationError::UnexpectedEndOfInput:
        return "UnexpectedEndOfInput";
    case CompilationError::InvalidToken:
        return "InvalidToken";
    case CompilationError::SymbolRedefinition:
        return "SymbolRedefinition";
    case CompilationError::AliasRedefinition:
        return "AliasRedefinition";
    case CompilationError::UnknownIsr:
        return "UnknownIsr";
    case CompilationError::InvalidIsrArgument:
        return "InvalidIsrArgument";
    case CompilationError::SymbolIsNotAllowed:
        return "SymbolIsNotAllowed";
    case CompilationError::AliasIsNotAllowed:
        return "AliasIsNotAllowed";
    case CompilationError::InvalidOperandSize:
        return "InvalidOperandSize";
    case CompilationError::InvalidCommandArgument:
        return "InvalidCommandArgument";
    case CompilationError::OperandModeNotSupported:
        return "OperandModeNotSupported";
    case CompilationError::UnknownCommand:
        return "UnknownCommand";
    case CompilationError::InvalidOperandArgument:
        return "InvalidOperandArgument";
    }

    return fmt::format("Invalid error id {}", static_cast<int>(error));
}

std::string GetDefaultMessage(CompilationError error, const Token &t) {
    switch (error) {
    case CompilationError::NoError:
        return "No error";

    case CompilationError::Unknown:
        return "Unknown error";
    case CompilationError::InternalError:
        return "Internal compiler error";
    case CompilationError::UnknownCommand:
        return fmt::format("Unknown command '{}'", t.View());

    case CompilationError::InvalidEscapeSequence:
        return "Invalid escape sequence";
    case CompilationError::UnfinishedQuotedString:
        return "Quoted text was not terminated";

    case CompilationError::UnexpectedInput:
        return fmt::format("'{}' was not expected", t.View());
    case CompilationError::UnexpectedEndOfInput:
        return fmt::format("More input was expected after '{}'", t.View());
    case CompilationError::InvalidToken:
        return fmt::format("'{}' is not valid in its context", t.View());

    case CompilationError::SymbolRedefinition:
        return fmt::format("Symbol '{}' is already defined", t.View());
    case CompilationError::AliasRedefinition:
        return fmt::format("Alias '{}' is already defined", t.View());

    case CompilationError::UnknownIsr:
        return fmt::format("Unknown isr '{}'", t.View());
    case CompilationError::InvalidIsrArgument:
        return fmt::format("'{}' is not valid isr argument", t.View());

    case CompilationError::SymbolIsNotAllowed:
        return fmt::format("Symbol '{}' is not allowed", t.View());
    case CompilationError::AliasIsNotAllowed:
        return fmt::format("Alias '{}' is not allowed", t.View());

    case CompilationError::InvalidOperandSize:
        return fmt::format("Invalid operand size");
    case CompilationError::InvalidOperandArgument:
        return fmt::format("Invalid operand argument");
    case CompilationError::OperandModeNotSupported:
        return fmt::format("Invalid operand address mode");
    case CompilationError::InvalidCommandArgument:
        return fmt::format("Invalid operand argument");
    }

    return fmt::format("Unknown message for code {}", static_cast<int>(error));
}

std::string CompilationException::Message() const {
    if (token.has_value()) {
        return fmt::format("{} : {} : {}", to_string(*token), to_string(error), what());
    } else {
        return fmt::format("{} : {}", to_string(error), what());
    }
}

} // namespace emu::emu6502::assembler
