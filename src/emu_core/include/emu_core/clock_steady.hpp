#pragma once

#include "emu_core/clock.hpp"
#include <chrono>
#include <cstdint>
#include <fmt/format.h>
#include <iostream>
#include <thread>

namespace emu {

struct ClockSteadyException : public std::runtime_error {
    ClockSteadyException(const std::string &msg) : runtime_error(msg) {}
};

//Single thread only
struct ClockSteady final : public Clock {
    static constexpr uint64_t kMaxFrequency = 100'000'000llu;
    static constexpr uint64_t kNanosecondsPerSecond = 1'000'000'000llu;

    using steady_clock = std::chrono::steady_clock;

    ClockSteady(uint64_t frequency = k1MhzFrequency,
                std::ostream *verbose_stream = nullptr)
        : frequency{frequency}, tick{kNanosecondsPerSecond / frequency}
    //   ,verbose_stream(verbose_stream)
    {
        if (frequency > kMaxFrequency) {
            throw ClockSteadyException("ClockSteady: frequency > kMaxFrequency");
        }
        ClockSteady::Reset();
        (void)verbose_stream;
    }

    [[nodiscard]] uint64_t CurrentCycle() const override { return current_cycle; }

    void WaitForNextCycle() override {
        if (steady_clock::now() > next_cycle) {
            // if (verbose_stream != nullptr) {
            //     (*verbose_stream) << fmt::format("Lost cycle at {}\n", current_cycle);
            // }
            next_cycle += tick;
            ++lost_cycles;
            ++current_cycle;
            return;
        }

        ++current_cycle;

        while (next_cycle > steady_clock::now()) {
            // busy loop
            // putting thread to sleep is not precise enough
        }

        next_cycle += tick;
    }

    void Reset() override {
        current_cycle = 0;
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
    uint64_t current_cycle = 0;
    const uint64_t frequency;
    std::chrono::nanoseconds const tick;
    steady_clock::time_point next_cycle{};
    steady_clock::time_point start_time{};
    uint64_t lost_cycles = 0;
    // std::ostream *const verbose_stream;
};

} // namespace emu
