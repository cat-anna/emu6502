#include "args.hpp"
#include "emu_6502/assembler/compiler.hpp"
#include "emu_6502/cpu/cpu.hpp"
#include "emu_core/plugins/plugin_loader.hpp"
#include "runner.hpp"
#include <array>
#include <chrono>
#include <memory>

int main(int argc, char **argv) {
    using namespace emu::runner;
    using namespace emu::plugins;
    namespace fs = std::filesystem;

    try {
        auto plugin_loader =
            PluginLoader::CreateDynamic(fs::absolute(fs::path(*argv)).parent_path());
        auto runner = std::make_shared<Runner>(plugin_loader->GetDeviceFactory());
        auto args = ParseComandline(argc, argv);
        runner->Setup(args);
        return runner->Start();
    } catch (const std::exception &e) {
        std::cerr << "ERROR: " << e.what() << "\n";
    }
    return 1;
}
