#include "args.hpp"
#include <emu_core/boost_po_utils.hpp>
#include <filesystem>
#include <fmt/format.h>
#include <iostream>
#include <stdexcept>

namespace emu::emu6502::assembler {

namespace fs = std::filesystem;

namespace po = boost::program_options;
using namespace emu::program_options;

namespace {

const ConflictingOptionsVec kConflictingOptions = {
    // {"code", "image"},
};

struct Options {
    po::options_description all_options;
    po::options_description cpu_options{"CPU options"};
    po::options_description input_options{"input options"};
    po::options_description output_options{"output options"};
    po::positional_options_description input_positional_opt;

    Options() {
        // clang-format off

        all_options.add_options()
            ("help", "Produce help message")
            ("verbose,v", "Print diagnostic logs during compilation")
            ;

        cpu_options.add_options()
            // ("cpu", po::value<uint64_t>()->default_value(1'000'000), "CPU clock speed in Hz. Use 0 for unlimited.")
            ;

        input_positional_opt.add("input", -1);
        input_options.add_options()
            ("input", po::value<std::string>()->required(), "Source file")
            //   ("include-path,I", po::value< vector<string> >(),
            ;

        output_options.add_options()
            ("bin-output", po::value<std::string>(), "Store binary image")
            ("hex-dump", po::value<std::string>(), "Write hex dump")
            ;

        // clang-format on

        all_options             //
            .add(cpu_options)   //
            .add(input_options) //
            .add(output_options);
    }

    ExecArguments ParseComandline(int argc, char **argv) {
        try {
            po::variables_map vm;
            po::store(po::command_line_parser(argc, argv) //
                          .options(all_options)
                          .positional(input_positional_opt)
                          .run(),
                      vm);
            po::notify(vm);
            if (vm.count("help") > 0) {
                PrintHelp(0);
            }
            ConflictingOptions(vm, kConflictingOptions);
            ExecArguments exec_args;
            ReadVariableMap(vm, exec_args);
            return exec_args;
        } catch (const std::logic_error &e) {
            std::cout << "Error: " << e.what() << "\n";
            std::cout << "\n";
            PrintHelp(1);
        }
    }

protected:
    void ReadVariableMap(const po::variables_map &vm, ExecArguments &args) {
        args.verbose = vm.count("verbose") > 0;

        ReadInputOptions(args.streams, args.input_options, vm);
        ReadOutputOptions(args.streams, args.output_options, vm);
    }

    void ReadInputOptions(StreamContainer &streams, ExecArguments::Input &opts, const po::variables_map &vm) {
        if (vm.count("input") != 1) {
            throw std::logic_error("Single input file is required");
        }
        auto input = vm["input"].as<std::string>();
        opts.input_name = input == "-" ? std::string("stdin") : input;
        opts.input = streams.OpenTextInput(input);
    }

    void ReadOutputOptions(StreamContainer &streams, ExecArguments::Output &opts, const po::variables_map &vm) {
        if (vm.count("bin-output") > 0) {
            opts.binary_output = streams.OpenBinaryOutput(vm["bin-output"].as<std::string>());
        }
        if (vm.count("hex-dump") > 0) {
            opts.hex_dump = streams.OpenBinaryOutput(vm["hex-dump"].as<std::string>());
        }
    }

    // CodeLoadOptions ReadCodeMode(const po::variables_map &vm) {
    //     CodeLoadOptions clo;
    //     clo.source_file = vm["code"].as<std::string>();
    //     return clo;
    // }

    // ExecutionOptions ReadExecutionOptions(const po::variables_map &vm) {
    //     ExecutionOptions eo;
    //     eo.frequency = vm["frequency"].as<uint64_t>();
    //     return eo;
    // }

    [[noreturn]] void PrintHelp(int exit_code) {
        std::cout << "Emu 6502 assembly compiler";
        std::cout << "\n";
        std::cout << all_options;
        exit(exit_code);
    }
};

} // namespace

ExecArguments ParseComandline(int argc, char **argv) {
    return Options().ParseComandline(argc, argv);
}

} // namespace emu::emu6502::assembler
