#include "args.hpp"
#include <boost/filesystem/string_file.hpp>
#include <boost/program_options.hpp>
#include <emu_core/boost_po_utils.hpp>
#include <emu_core/file_search.hpp>
#include <emu_core/string_file.hpp>
#include <filesystem>
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

    std::shared_ptr<FileSearch> file_search =
        FileSearch::CreateFromEnv("EMU6502_CONFIG_PATH");

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
            ("image", po::value<std::vector<std::string>>(), "Image to run")
            ("config", po::value<std::vector<std::string>>(), "Config to run")
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

    void ReadMemoryOptions(StreamContainer &streams, MemoryConfig &opts,
                           const po::variables_map &vm) {
        opts.entries.clear();

        bool was_binary = false;
        bool was_yaml = false;

        auto load_entry = [&](auto image_file) {
            auto ext = std::filesystem::path(image_file).extension().generic_string();
            if (ext == ".yaml") {
                if (was_binary) {
                    throw std::logic_error(
                        "Binary files cannot be loaded together with yaml configs");
                }
                was_yaml = true;
                auto conf =
                    LoadMemoryConfigurationFromFile(image_file, file_search.get());
                opts.entries.insert(opts.entries.end(), conf.entries.begin(),
                                    conf.entries.end());
            } else {
                if (was_yaml || !opts.entries.empty()) {
                    throw std::logic_error(
                        "Binary files cannot be loaded together with yaml configs");
                }
                was_binary = true;

                MemoryConfigEntry area;
                area.offset = 0;
                area.name = image_file;
                MemoryConfigEntry::RamArea ram_area{
                    .image = MemoryConfigEntry::RamArea::Image{.file = image_file,
                                                               .offset = 0},
                    .writable = true,
                };
                area.entry_variant = ram_area;
                opts.entries.emplace_back(area);
            }
        };

        if (vm.count("image") > 0) {
            for (auto image_file : vm["image"].as<std::vector<std::string>>()) {
                load_entry(image_file);
            }
        }

        if (vm.count("config") > 0) {
            for (auto image_file : vm["config"].as<std::vector<std::string>>()) {
                load_entry(image_file);
            }
        }

        if (opts.entries.empty()) {
            throw std::logic_error("Image/config to run is not provided");
        }
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
