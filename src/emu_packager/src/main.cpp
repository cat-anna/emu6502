#include "args.hpp"
#include "runner.hpp"
#include <array>
#include <chrono>
#include <iostream>
#include <memory>

int main(int argc, char *argv[]) {
    using namespace emu::packager;
    namespace fs = std::filesystem;

    try {
        auto runner = std::make_shared<Runner>();
        auto args = ParseComandline(argc, argv);
        return runner->Pack(args);
    } catch (const std::exception &e) {
        std::cerr << "ERROR: " << e.what() << "\n";
    }
    return 1;
}
