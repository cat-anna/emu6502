#include "args.hpp"
#include "runner.hpp"

int main(int argc, char **argv) {
    using namespace emu::emu6502::assembler;
    return std::make_shared<Runner>()->Start(ParseComandline(argc, argv));
}
