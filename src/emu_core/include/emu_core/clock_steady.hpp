#pragma once

#include "emu_core/clock.hpp"
#include <chrono>
#include <cstdint>
#include <fmt/format.h>
#include <iostream>
#include <thread>

namespace emu {

constexpr uint64_t kNesFrequency =
    1'789'773llu; //TODO: check which frequency it is and name it correctly

constexpr uint64_t k1MhzFrequency = 1'000'000llu;
constexpr uint64_t k1KhzFrequency = 1'000llu;

struct ClockSteadyException : public std::runtime_error {
    ClockSteadyException(const std::string &msg) : runtime_error(msg) {}
};

//Single thread only
struct ClockSteady : public ClockSimple {
    using steady_clock = std::chrono::steady_clock;
    static constexpr uint64_t kMaxFrequency = 100'000'000llu;
    static constexpr uint64_t kNanosecondsPerSecond = 1'000'000'000llu;

    ClockSteady(uint64_t frequency = k1MhzFrequency)
        : frequency{frequency}, tick{kNanosecondsPerSecond / frequency}, next_cycle{} {
        if (frequency > kMaxFrequency) {
            throw ClockSteadyException("ClockSteady: frequency > kMaxFrequency");
        }
        Reset();
    }

    void WaitForNextCycle() override {
        if (steady_clock::now() > next_cycle) {
            next_cycle = steady_clock::now() + tick;
            ++lost_cycles;
            ClockSimple::WaitForNextCycle();
            return;
        }

        ClockSimple::WaitForNextCycle();

        while (next_cycle > steady_clock::now()) {
            // busy loop
            // putting thread to sleep is not precise enough
        }

        next_cycle += tick;
    }

    void Reset() override {
        ClockSimple::Reset();
        start_time = steady_clock::now();
        next_cycle = start_time + tick;
    }

    [[nodiscard]] uint64_t LostCycles() const override { return lost_cycles; }
    [[nodiscard]] uint64_t Frequency() const override { return frequency; }

    [[nodiscard]] double Time() const override {
        std::chrono::duration<double> dt = steady_clock::now() - start_time;
        return dt.count();
    };

private:
    const uint64_t frequency;
    std::chrono::nanoseconds const tick;
    steady_clock::time_point next_cycle;
    steady_clock::time_point start_time;
    uint64_t lost_cycles = 0;
};

} // namespace emu
