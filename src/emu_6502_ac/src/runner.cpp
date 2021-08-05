#include "runner.hpp"
#include "emu_6502/assembler/compilation_error.hpp"
#include "emu_core/text_utils.hpp"
#include <iostream>
#include <sstream>

namespace emu::emu6502::assembler {

int Runner::Start(const ExecArguments &exec_args) {
    verbose = exec_args.verbose;

    try {
        auto compiler = InitCompiler(exec_args);

        for (auto &input : exec_args.input_options) {
            compiler->Compile(*input.stream, input.name);
        }

        auto program = compiler->GetProgram();

        StoreOutput(exec_args.output_options, *program);
        return 0;
    } catch (const CompilationException &e) {
        std::cout << "Error: " << e.Message() << "\n";
        std::cout << e.Location().GetDescription();
        return static_cast<int>(e.Error());
    } catch (const std::exception &e) {
        std::cout << "Error: " << e.what() << "\n";
        return -1;
    }
}

std::unique_ptr<Compiler6502> Runner::InitCompiler(const ExecArguments &exec_args) {
    auto compiler = std::make_unique<Compiler6502>(exec_args.cpu_options.instruction_set,
                                                   verbose ? &std::cout : nullptr);

    auto symbols = symbol_factory->GetSymbols(exec_args.memory_options);
    compiler->AddDefinitions(symbols);

    return compiler;
}

void Runner::StoreOutput(const ExecArguments::Output &output_options, Program &program) {
    if (output_options.binary_output != nullptr) {
        auto bin_data = program.sparse_binary_code.DumpMemory();
        output_options.binary_output->write(reinterpret_cast<const char *>(&bin_data[0]),
                                            bin_data.size());
    }
    if (output_options.hex_dump != nullptr) {
        auto hex = program.sparse_binary_code.HexDump();
        *output_options.hex_dump << hex;
    }

    if (output_options.symbol_dump != nullptr) {
        auto out = GenerateSymbolDump(program);
        *output_options.symbol_dump << out;
    }
}

} // namespace emu::emu6502::assembler
