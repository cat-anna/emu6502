#include "emu_6502/assembler/compilation_error.hpp"

namespace emu::emu6502::assembler {

std::string to_string(CompilationError error) {
    switch (error) {
    case CompilationError::Unknown:
        return "Unknown";
    case CompilationError::UnexpectedInput:
        return "UnexpectedInput";
    case CompilationError::UnexpectedEndOfInput:
        return "UnexpectedEndOfInput";
    case CompilationError::InvalidToken:
        return "InvalidToken";
    case CompilationError::LabelRedefinition:
        return "LabelRedefinition";
    case CompilationError::AliasRedefinition:
        return "AliasRedefinition";
    case CompilationError::UnknownIsr:
        return "UnknownIsr";
    case CompilationError::InvalidIsrArgument:
        return "InvalidIsrArgument";
    case CompilationError::LabelIsNotAllowed:
        return "LabelIsNotAllowed";
    case CompilationError::AliasIsNotAllowed:
        return "AliasIsNotAllowed";
    case CompilationError::InvalidOperandSize:
        return "InvalidOperandSize";
    }

    return fmt::format("Invalid error id {}", static_cast<int>(error));
}

std::string GetDefaultMessage(CompilationError error, const Token &t) {
    switch (error) {
        // case CompilationError::Unknown:
        // return "Unknown";

    case CompilationError::UnexpectedInput:
        return fmt::format("'{}' was not expected", t.View());
    case CompilationError::UnexpectedEndOfInput:
        return fmt::format("More input was expected after '{}'", t.View());
    case CompilationError::InvalidToken:
        return fmt::format("'{}' is not valid in its context", t.View());
    case CompilationError::LabelRedefinition:
        return fmt::format("Label '{}' is already defined", t.View());
    case CompilationError::AliasRedefinition:
        return fmt::format("Alias '{}' is already defined", t.View());
    case CompilationError::UnknownIsr:
        return fmt::format("Unknown isr '{}'", t.View());

        //TODO:

        // case CompilationError::InvalidIsrArgument:
        //     return "InvalidIsrArgument";
        // case CompilationError::LabelIsNotAllowed:
        //     return "LabelIsNotAllowed";
        // case CompilationError::AliasIsNotAllowed:
        //     return "AliasIsNotAllowed";
        // case CompilationError::InvalidOperandSize:
        //     return "InvalidOperandSize";
    }

    return fmt::format("TODO CompilationError message {}", static_cast<int>(error));
}

std::string CompilationException::Message() const {
    return fmt::format("{} : {} : {}", to_string(token), to_string(error), what());
}

} // namespace emu::emu6502::assembler
