#include "args.hpp"
#include <boost/filesystem/string_file.hpp>
#include <boost/program_options.hpp>
#include <emu_core/boost_po_utils.hpp>
#include <emu_core/string_file.hpp>
#include <fmt/format.h>
#include <iostream>
#include <stdexcept>

namespace emu::runner {

namespace fs = std::filesystem;

namespace po = boost::program_options;
using namespace emu::program_options;

namespace {

const ConflictingOptionsVec kConflictingOptions = {};

struct Options {
    po::options_description all_options;
    po::options_description cpu_options{"Cpu options"};
    po::options_description image_options{"Image load options"};
    po::positional_options_description image_positional_opt;

    Options() {
        // clang-format off

        all_options.add_options()
            ("help", "Produce help message")
            ("verbose,v", "Print diagnostic logs during execution")
            ;

        cpu_options.add_options()
            ("frequency", po::value<uint64_t>()->default_value(1'000'000), "CPU clock speed in Hz. Use 0 for unlimited.")
            // ("cpu", po::value<uint64_t>()->default_value(1'000'000), "CPU clock speed in Hz. Use 0 for unlimited.")
            ;

        image_positional_opt.add("image", -1);
        image_options.add_options()
            ("image", po::value<std::string>(), "Image to run")
            ;

        // clang-format on

        all_options             //
            .add(cpu_options)   //
            .add(image_options) //
            ;
    }

    ExecArguments ParseComandline(int argc, char **argv) {
        try {
            po::variables_map vm;
            po::store(po::command_line_parser(argc, argv) //
                          .options(all_options)
                          .positional(image_positional_opt)
                          .run(),
                      vm);
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

        ReadCpuOptions(args.streams, args.cpu_options, vm);
        ReadMemoryOptions(args.streams, args.memory_options, vm);
    }

    void ReadCpuOptions(StreamContainer &streams, ExecArguments::CpuOptions &opts,
                        const po::variables_map &vm) {
        opts.frequency = vm["frequency"].as<uint64_t>();
    }

    void ReadMemoryOptions(StreamContainer &streams, ExecArguments::MemoryOptions &opts,
                           const po::variables_map &vm) {
        if (vm.count("image") != 1) {
            throw std::logic_error("Image to run is not provided");
        }

        auto image_file = vm["image"].as<std::string>();

        opts.clear();
        ExecArguments::MemoryArea area;
        area.offset = 0;
        area.block = ExecArguments::MemoryArea::MemoryBlock{
            .bytes = load_file<std::vector<uint8_t>>(image_file),
            .rw = true,
        };
        opts.emplace_back(area);
    }

    [[noreturn]] void PrintHelp(int exit_code) {
        std::cout << "Emu 6502 runner";
        std::cout << "\n";
        std::cout << all_options;
        exit(exit_code);
    }
};

} // namespace

ExecArguments ParseComandline(int argc, char **argv) {
    return Options().ParseComandline(argc, argv);
}

} // namespace emu::runner
