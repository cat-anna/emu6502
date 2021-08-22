#include "args.hpp"
#include "emu_core/boost_po_utils.hpp"
#include "emu_core/clock.hpp"
#include "emu_core/file_search.hpp"
#include "emu_core/package/package_builder.hpp"
#include "emu_core/package/package_builder_zip.hpp"
#include "emu_core/string_file.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/string_file.hpp>
#include <boost/program_options.hpp>
#include <filesystem>
#include <fmt/format.h>
#include <iostream>
#include <stdexcept>

namespace emu::packager {

namespace fs = std::filesystem;

namespace po = boost::program_options;
using namespace emu::program_options;
using namespace emu::package;

namespace {

struct Options {
    po::options_description all_options;
    po::options_description image_options{"Image options"};
    po::options_description out_options{"Output options"};
    po::positional_options_description arg_positional_opt;

    std::shared_ptr<FileSearch> file_search = FileSearch::CreateDefault();

    std::unordered_map<std::string, std::string> value_overrides;

    Options() {
        // clang-format off

        all_options.add_options()
            ("help", "Produce help message")
            // ("verbose-base,v", "Print base diagnostic logs during execution")
            // ("verbose", po::value<std::string>(), "Print some diagnostic messages")
            ("override", po::value<std::vector<std::string>>(), "Override text value referenced by config. Use form KEY=VALUE")
            ;

        out_options.add_options()
            ("output,o", po::value<std::string>()->required(), "Output file")
            ;

        // arg_positional_opt.add("arg", -1);
        image_options.add_options()
            ("config", po::value<std::string>()->required(), "Config to pack")
            ;

        // clang-format on

        all_options             //
            .add(out_options)   //
            .add(image_options) //
            ;
    }

    ExecArguments ParseComandline(int argc, char **argv) {
        try {
            po::variables_map vm;
            po::store(po::command_line_parser(argc, argv) //
                          .options(all_options)
                          //   .positional(arg_positional_opt)
                          .run(),
                      vm);
            if (vm.count("help") > 0) {
                PrintHelp(0);
            }
            // ConflictingOptions(vm, kConflictingOptions);
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
        if (vm.count("override") > 0) {
            for (const auto &item : vm["override"].as<std::vector<std::string>>()) {
                std::vector<std::string> result;
                boost::split(result, item, boost::is_any_of("="));
                switch (result.size()) {
                case 1:
                    value_overrides[std::to_string(value_overrides.size())] = result[0];
                    continue;
                case 2:
                    value_overrides[result[0]] = result[1];
                    continue;
                default:
                    throw std::runtime_error("Incorrect override value " + item);
                }
            }
        }

        ReadOutputOptions(args.streams, args, vm);
        ReadConfigOptions(args.streams, args.memory_options, vm);
    }

    void ReadOutputOptions(StreamContainer &streams, ExecArguments &opts,
                           const po::variables_map &vm) {
        if (vm.count("output") != 1) {
            throw std::runtime_error("Output is not specified");
        }

        opts.output_path = vm["output"].as<std::string>();
        if (!opts.output_path.ends_with(kEmuImageExtension)) {
            opts.output_path += kEmuImageExtension;
        }
    }

    void ReadConfigOptions(StreamContainer &streams, MemoryConfig &opts,
                           const po::variables_map &vm) {
        opts.entries.clear();

        if (vm.count("config") > 0) {
            auto &image_file = vm["config"].as<std::string>();
            auto conf = LoadMemoryConfigurationFromFile(image_file, file_search.get(),
                                                        value_overrides);
            opts.entries.insert(opts.entries.end(), conf.entries.begin(),
                                conf.entries.end());
        }

        if (opts.entries.empty()) {
            throw std::logic_error("Config to use was not provided");
        }
    }

    [[noreturn]] void PrintHelp(int exit_code) const {
        std::cout << "Emu packager";
        std::cout << "\n";
        std::cout << all_options;
        std::cout << "\n";
        exit(exit_code);
    }
};

} // namespace

ExecArguments ParseComandline(int argc, char **argv) {
    return Options().ParseComandline(argc, argv);
}

} // namespace emu::packager
