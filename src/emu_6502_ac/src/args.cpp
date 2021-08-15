#include "args.hpp"
#include <emu_core/boost_po_utils.hpp>
#include <emu_core/file_search.hpp>
#include <filesystem>
#include <fmt/format.h>
#include <iostream>
#include <stdexcept>

namespace emu::emu6502::assembler {

namespace fs = std::filesystem;

namespace po = boost::program_options;
using namespace emu::program_options;
using namespace std::string_literals;

namespace {

const ConflictingOptionsVec kConflictingOptions = {
    // {"code", "image"},
};

struct Options {
    po::options_description all_options;
    po::options_description cpu_options{"CPU options"};
    po::options_description input_options{"input options"};
    po::options_description output_options{"output options"};
    po::options_description memory_options{"memory options"};
    po::positional_options_description positional_opt;

    std::shared_ptr<FileSearch> file_search = FileSearch::CreateDefault();

    Options() {
        // clang-format off

        all_options.add_options()
            ("help", "Produce help message")
            ("verbose,v", "Print diagnostic logs during compilation")
            ;

        cpu_options.add_options()
            // ("cpu", po::value<uint64_t>()->default_value(1'000'000), "CPU clock speed in Hz. Use 0 for unlimited.")
            ;

        memory_options.add_options()
            ("config", po::value<std::vector<std::string>>(), "Memory configuration file")
            ;

        positional_opt.add("input", -1);
        input_options.add_options()
            ("input", po::value<std::vector<std::string>>(), "Source file")
            //   ("include-path,I", po::value< vector<string> >(),
            ;

        output_options.add_options()
            ("bin-output", po::value<std::string>(), "Store binary image")
            ("hex-dump", po::value<std::string>(), "Write hex dump")
            ("symbol-dump", po::value<std::string>(), "Write symbols")
            ;

        // clang-format on

        all_options              //
            .add(cpu_options)    //
            .add(input_options)  //
            .add(output_options) //
            .add(memory_options);
    }

    ExecArguments ParseComandline(int argc, char **argv) {
        try {
            po::variables_map vm;
            po::store(po::command_line_parser(argc, argv) //
                          .options(all_options)
                          .positional(positional_opt)
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
        ReadMemoryOptions(args.streams, args.memory_options, vm);
    }

    void ReadInputOptions(StreamContainer &streams,
                          std::vector<ExecArguments::Input> &opts,
                          const po::variables_map &vm) {
        opts.clear();
        if (vm.count("input") > 0) {
            for (auto &input_file : vm["input"].as<std::vector<std::string>>()) {
                auto input_name = input_file == "-" ? "stdin"s : input_file;
                auto input = streams.OpenTextInput(input_file);
                opts.emplace_back(ExecArguments::Input{input_name, input});
            }
        }
    }

    void ReadOutputOptions(StreamContainer &streams, ExecArguments::Output &opts,
                           const po::variables_map &vm) {
        if (vm.count("bin-output") > 0) {
            opts.binary_output =
                streams.OpenBinaryOutput(vm["bin-output"].as<std::string>());
        }
        if (vm.count("hex-dump") > 0) {
            opts.hex_dump = streams.OpenTextOutput(vm["hex-dump"].as<std::string>());
        }
        if (vm.count("symbol-dump") > 0) {
            opts.symbol_dump =
                streams.OpenTextOutput(vm["symbol-dump"].as<std::string>());
        }
    }

    void ReadMemoryOptions(StreamContainer &streams, MemoryConfig &opts,
                           const po::variables_map &vm) {
        opts.entries.clear();

        if (vm.count("config") > 0) {
            for (auto &file : vm["config"].as<std::vector<std::string>>()) {
                auto conf = LoadMemoryConfigurationFromFile(file, file_search.get());
                opts.entries.insert(opts.entries.end(), conf.entries.begin(),
                                    conf.entries.end());
            }
        }
    }

    [[noreturn]] void PrintHelp(int exit_code) const {
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
