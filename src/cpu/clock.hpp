#pragma once

#include <cstdint>
#include <fmt/format.h>
#include <iostream>

namespace emu6502::cpu {

struct Clock {

    using Clock_t = uint64_t;

    Clock_t CurrentCycle() const { return current_cycle; }

    void WaitForNextCycle() {
        // std::cout << fmt::format("TICK {}\n", current_cycle);
        ++current_cycle; //TODO
    }

private:
    Clock_t current_cycle = 0;
};

} // namespace emu6502::cpu