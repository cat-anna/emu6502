#pragma once

#include <cstdint>

namespace emu {

struct Clock {
    virtual ~Clock() = default;
    virtual void WaitForNextCycle() = 0;
    virtual void Reset() = 0;

    virtual uint64_t CurrentCycle() const { return 0; };
    virtual uint64_t Frequency() const { return 0; };
    virtual uint64_t LostCycles() const { return 0; };
};

struct ClockSimple : public Clock {
    using Clock_t = uint64_t;

    void WaitForNextCycle() override { ++current_cycle; }
    void Reset() override { current_cycle = 0; }

    Clock_t CurrentCycle() const override { return current_cycle; }

private:
    Clock_t current_cycle = 0;
};

} // namespace emu