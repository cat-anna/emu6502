#include "emu_6502/cpu/cpu.hpp"
#include "emu_core/package/package_builder.hpp"
#include "emu_core/package/package_zip.hpp"
#include "emu_core/plugins/plugin_loader.hpp"
#include "emu_core/simulation/simulation_builder.hpp"
#include "gtest/gtest.h"
#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/scope_exit.hpp>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

struct TestCase {
    std::string name;
    std::string image;
};

std::filesystem::path executable_path =
    std::filesystem::absolute(
        std::filesystem::path(boost::dll::program_location().generic_string()))
        .parent_path();

std::filesystem::path images_base_path = executable_path / "functional_test_images";

std::vector<TestCase> FindTestCases() {
    using namespace emu;

    std::cout << images_base_path.generic_string() << "\n";
    if (!std::filesystem::is_directory(images_base_path)) {
        return {};
    }

    std::vector<TestCase> r;
    for (auto it = std::filesystem::directory_iterator(images_base_path);
         it != std::filesystem::directory_iterator(); ++it) {
        auto file_name = it->path().generic_string();
        if (file_name.ends_with(package::kEmuImageExtension)) {
            auto name = it->path().stem().generic_string();
            if (name.ends_with("_image")) {
                name.resize(name.size() - strlen("_image"));
            }
            r.emplace_back(TestCase{
                .name = name,
                .image = file_name,
            });
        }
    }
    return r;
}

class FunctionalTest : public ::testing::TestWithParam<TestCase> {};

TEST_P(FunctionalTest, ) {
    using namespace emu;
    using namespace emu::plugins;
    namespace fs = std::filesystem;

    auto plugin_loader = PluginLoader::CreateDynamic(executable_path);
    auto device_factory = plugin_loader->GetDeviceFactory();

    const auto &test_param = GetParam();
    auto package = std::make_unique<package::ZipPackage>(test_param.image);

    // auto vc = SimulationBuildVerboseConfig::Stdout();
    auto vc = SimulationBuildVerboseConfig{};
    vc.memory = nullptr;
    vc.memory_mapper = nullptr;

    auto cpu = SimulationBuildCpuConfig{
        .frequency = 0,
        .instruction_set = emu6502::InstructionSet::NMOS6502Emu,
    };

    auto simulation = BuildEmuSimulation(device_factory, package.get(), cpu, vc);

    std::optional<EmuSimulation::Result> result;

    EXPECT_NO_THROW({
        try {
            result = simulation->Run();
        } catch (const EmuSimulation::SimulationFailedException &e) {
            std::cout << "FATAL: " << e.what() << "\n";
            result = e.GetResult();
            throw;
        } catch (const std::exception &e) {
            std::cout << "FATAL: " << e.what() << "\n";
            throw;
        }
    });

    EXPECT_TRUE(result.has_value());
    if (!result.has_value()) {
        return;
    }
    std::string halt_code = "-";
    if (result->halt_code.has_value()) {
        halt_code = std::to_string(result->halt_code.value_or(0));
    }
    std::cout << fmt::format("Halt code {}\n", halt_code);
    std::cout << fmt::format("Took {:.6f} seconds\n", result->duration);
    std::cout << fmt::format("Cpu cycles: {} ({:.3f} Hz)\n", result->cpu_cycles,
                             static_cast<double>(result->cpu_cycles) / result->duration);

    EXPECT_EQ(result->halt_code.value_or(0u), 0u);
}

auto GetTestName() {
    return [](auto &info) { return info.param.name; };
}

INSTANTIATE_TEST_SUITE_P(, FunctionalTest, ::testing::ValuesIn(FindTestCases()),
                         GetTestName());

int main(int argc, char **argv) {
    srand(static_cast<unsigned>(time(nullptr)));
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
