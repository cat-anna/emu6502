#include "args.hpp"
#include "emu_6502/assembler/compiler.hpp"
#include "emu_6502/cpu/cpu.hpp"
#include "runner.hpp"
#include <array>
#include <chrono>

#include <memory>

int main(int argc, char **argv) {
    using namespace emu::runner;
    Runner runner;
    return runner.Setup(ParseComandline(argc, argv)).Start();
}
