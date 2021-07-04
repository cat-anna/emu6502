#include "args.hpp"
#include "emu_core/plugins/plugin_loader.hpp"
#include "runner.hpp"
#include <filesystem>

int main(int argc, char **argv) {
    using namespace emu::emu6502::assembler;
    using namespace emu::plugins;
    namespace fs = std::filesystem;
    try {
        auto plugin_loader =
            PluginLoader::CreateDynamic(fs::absolute(fs::path(argv[0])).parent_path());
        auto runner = std::make_shared<Runner>(plugin_loader->GetSymbolFactory());
        return runner->Start(ParseComandline(argc, argv));
    } catch (const std::exception &e) {
        std::cerr << "ERROR: " << e.what() << "\n";
    }
}
