#include "args.hpp"
#include "emu_core/boost_po_utils.hpp"
#include "emu_core/clock.hpp"
#include "emu_core/file_search.hpp"
#include "emu_core/package/package_builder.hpp"
#include "emu_core/package/package_fs.hpp"
#include "emu_core/package/package_zip.hpp"
#include "emu_core/string_file.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/string_file.hpp>
#include <boost/program_options.hpp>
#include <filesystem>
#include <fmt/format.h>
#include <iostream>
#include <stdexcept>

namespace emu::runner {

namespace fs = std::filesystem;

namespace po = boost::program_options;
using namespace emu::program_options;

namespace {

const std::unordered_map<std::string, std::set<Verbose>> kVerboseAreas = {
    {
        "all",
        {
            Verbose::Memory,
            Verbose::MemoryMapper,
            Verbose::Result,
            Verbose::Cpu,
            Verbose::Clock,
            Verbose::Device,
        },
    },
    {"base", {Verbose::Result, Verbose::Cpu, Verbose::Clock, Verbose::Device}},
    {"device", {Verbose::Device}},
    {"result", {Verbose::Result}},
    {"cpu", {Verbose::Cpu}},
    {"clock", {Verbose::Clock}},
    {"memory", {Verbose::Memory}},
    {"memorymapper", {Verbose::MemoryMapper}},
};

const ConflictingOptionsVec kConflictingOptions = {};

struct Options {
    po::options_description all_options;
    po::options_description cpu_options{"Cpu options"};
    po::options_description image_options{"Image load options"};
    po::positional_options_description image_positional_opt;

    std::shared_ptr<FileSearch> file_search = FileSearch::CreateDefault();

    Options() {
        // clang-format off

        all_options.add_options()
            ("help", "Produce help message")
            ("verbose-base,v", "Print base diagnostic logs during execution")
            ("verbose", po::value<std::string>(), "Print some diagnostic messages")
            ("verbose-out", po::value<std::string>(), "Verbose output. Default is stdout")
            ;

        cpu_options.add_options()
            ("frequency", po::value<uint64_t>()->default_value(emu::k1MhzFrequency), "CPU clock speed in Hz. Use 0 for unlimited.")
            // ("cpu", po::value<uint64_t>()->default_value(1'000'000), "CPU clock speed in Hz. Use 0 for unlimited.")
            ;

        image_positional_opt.add("image", -1);
        image_options.add_options()
            ("image", po::value<std::string>()->required(), "Image to run")
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
        std::set<std::string> verbose_areas;
        if (vm.count("verbose-out") > 0) {
            args.verbose_stream =
                args.streams.OpenTextOutput(vm["verbose-out"].as<std::string>());
        }
        if (vm.count("verbose-base") > 0) {
            verbose_areas.insert("base");
        }
        if (vm.count("verbose") > 0) {
            boost::split(verbose_areas,
                         boost::to_lower_copy(vm["verbose"].as<std::string>()),
                         boost::is_any_of(","));
        }

        for (const auto &item : verbose_areas) {
            if (!kVerboseAreas.contains(item)) {
                throw std::runtime_error(fmt::format("{} is not valid verbose area"));
            }
            for (auto i : kVerboseAreas.at(item)) {
                args.verbose.insert(i);
            }
        }

        ReadCpuOptions(args.streams, args.cpu_options, vm);
        OpenPackage(args, vm);

        if (!args.package) {
            throw std::logic_error("Image/config to run was not provided");
        }
    }

    void ReadCpuOptions(StreamContainer &streams, ExecArguments::CpuOptions &opts,
                        const po::variables_map &vm) {
        opts.frequency = vm["frequency"].as<uint64_t>();
    }

    void OpenPackage(ExecArguments &args, const po::variables_map &vm) {
        if (vm.count("image") != 1) {
            throw std::runtime_error("image path is not correct");
        }

        auto config = vm["image"].as<std::string>();

        auto path = std::filesystem::path(config);
        if (!std::filesystem::is_regular_file(path)) {
            throw std::runtime_error(fmt::format("file {} is not valid", config));
        }
        auto base_path = path.parent_path();
        auto ext = path.extension().generic_string();
        std::shared_ptr<FileSearch> searcher =
            file_search->PrependPath(base_path.generic_string());

        if (ext == ".yaml") {
            args.package = std::make_unique<package::FsPackage>(config, searcher);
            return;
        } else if (ext == package::kEmuImageExtension) {
            args.package = std::make_unique<package::ZipPackage>(config);
            return;
        } else {
            auto config_name = path.filename().generic_string();
            MemoryConfigEntry area;
            area.offset = 0;
            area.name = config;
            MemoryConfigEntry::RamArea ram_area{
                .image =
                    MemoryConfigEntry::RamArea::Image{.file = config_name, .offset = 0},
                .writable = true,
            };
            area.entry_variant = ram_area;
            MemoryConfig mem_config;
            mem_config.entries.emplace_back(area);
            args.package = std::make_unique<package::FsPackage>(mem_config, searcher);
            return;
        }
    }

    std::unique_ptr<package::IPackage> OpenFsPackage(const std::string &config,
                                                     ExecArguments &args,
                                                     const po::variables_map &vm) {}

    [[noreturn]] void PrintHelp(int exit_code) const {
        std::cout << "Emu 6502 runner";
        std::cout << "\n";
        std::cout << all_options;
        std::cout << "\n";
        std::cout << "Verbose areas: ";
        for (auto &[key, m] : kVerboseAreas) {
            std::cout << key << ",";
        }
        std::cout << "\n";
        exit(exit_code);
    }
};

} // namespace

std::ostream *ExecArguments::GetVerboseStream(Verbose v) const {
    if (verbose.contains(v)) {
        return verbose_stream;
    } else {
        return nullptr;
    }
}

ExecArguments ParseComandline(int argc, char **argv) {
    return Options().ParseComandline(argc, argv);
}

} // namespace emu::runner
