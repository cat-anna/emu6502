#pragma once

#include <cstdint>

namespace emu {

struct Clock {
    virtual ~Clock() = default;
    virtual void WaitForNextCycle() = 0;
    virtual void Reset() = 0;

    [[nodiscard]] virtual uint64_t CurrentCycle() const { return 0; };
    [[nodiscard]] virtual uint64_t Frequency() const { return 0; };
    [[nodiscard]] virtual uint64_t LostCycles() const { return 0; };

    [[nodiscard]] virtual double Time() const { return 0.0; };
};

struct ClockSimple : public Clock {
    using Clock_t = uint64_t;

    void WaitForNextCycle() override { ++current_cycle; }
    void Reset() override { current_cycle = 0; }

    [[nodiscard]] Clock_t CurrentCycle() const override { return current_cycle; }

private:
    Clock_t current_cycle = 0;
};

} // namespace emu

#ifdef WANTS_GTEST_MOCKS

#include <gmock/gmock.h>
namespace emu {
struct ClockMock : public Clock {
    MOCK_METHOD(void, WaitForNextCycle, ());
    MOCK_METHOD(void, Reset, ());
    MOCK_METHOD(uint64_t, CurrentCycle, (), (const));
    MOCK_METHOD(uint64_t, Frequency, (), (const));
    MOCK_METHOD(uint64_t, LostCycles, (), (const));
    MOCK_METHOD(double, Time, (), (const));
};

} // namespace emu

#endif
