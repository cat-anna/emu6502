#pragma once

#include "args.hpp"
#include "emu_6502/assembler/compiler.hpp"
#include "emu_core/symbol_factory.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace emu::emu6502::assembler {

struct Runner {
    Runner(std::shared_ptr<SymbolFactory> _symbol_factory)
        : symbol_factory(std::move(_symbol_factory)) {}
    int Start(const ExecArguments &exec_args);

protected:
    const std::shared_ptr<SymbolFactory> symbol_factory;
    bool verbose = false;

    std::unique_ptr<Compiler6502> InitCompiler(const ExecArguments &exec_args);

    void StoreOutput(const ExecArguments::Output &output_options, Program &program);
};

} // namespace emu::emu6502::assembler
