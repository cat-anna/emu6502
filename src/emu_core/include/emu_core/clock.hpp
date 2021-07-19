#pragma once

#include <chrono>
#include <cstdint>

namespace emu {

constexpr uint64_t kNesFrequency =
    1'789'773llu; //TODO: check which frequency it is and name it correctly

constexpr uint64_t k1MhzFrequency = 1'000'000llu;
constexpr uint64_t k1KhzFrequency = 1'000llu;

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
    void WaitForNextCycle() override { ++current_cycle; }
    void Reset() override { current_cycle = 0; }
    [[nodiscard]] uint64_t CurrentCycle() const override { return current_cycle; }
    [[nodiscard]] double Time() const override {
        return static_cast<double>(current_cycle);
    };

private:
    uint64_t current_cycle = 0;
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
