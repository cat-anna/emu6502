#pragma once

#include <cstdint>

namespace emu {

struct Clock {
    virtual ~Clock() = default;
    virtual void WaitForNextCycle() = 0;
};

struct ClockSimple : public Clock {
    using Clock_t = uint64_t;

    void WaitForNextCycle() override { ++current_cycle; }
    Clock_t CurrentCycle() const { return current_cycle; }

private:
    Clock_t current_cycle = 0;
};

} // namespace emu