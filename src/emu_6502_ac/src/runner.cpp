#include "runner.hpp"
#include "emu_6502/assembler/compilation_error.hpp"
#include "emu_6502/assembler/compiler.hpp"
#include <iostream>
// #include <emu_core/build_config.hpp>
// #include <emu_core/clock_steady.hpp>
// #include <emu_core/memory_mapper.hpp>
// #include <emu_core/memory_sparse.hpp>
// #include <emu_core/program.hpp>

namespace emu::emu6502::assembler {

int Runner::Start(const ExecArguments &exec_args) {
    verbose = exec_args.verbose;

    try {
        Compiler6502 c(exec_args.cpu_options.instruction_set, verbose);
        auto program = c.Compile(*exec_args.input_options.input, exec_args.input_options.input_name);

        if (exec_args.output_options.binary_output != nullptr) {
            auto bin_data = program->sparse_binary_code.DumpMemory();
            exec_args.output_options.binary_output->write(reinterpret_cast<const char *>(&bin_data[0]),
                                                          bin_data.size());
        }
        if (exec_args.output_options.hex_dump != nullptr) {
            auto hex = program->sparse_binary_code.HexDump();
            *exec_args.output_options.hex_dump << hex;
        }

        return 0;
    } catch (const CompilationException &e) {
        std::cout << "Error: " << e.Message() << "\n";
        // std::cout << "at " << to_string(e.Location()) << "\n";
        return static_cast<int>(e.Error());
    } catch (const std::exception &e) {
        std::cout << "Error: " << e.what() << "\n";
        return -1;
    }
}

} // namespace emu::emu6502::assembler
