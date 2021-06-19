#pragma once

#include "emu_core/clock.hpp"
#include "emu_core/memory.hpp"
#include <algorithm>
#include <array>
#include <concepts>
#include <cstdint>
#include <fmt/format.h>
#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace emu::memory {

template <std::unsigned_integral _Address_t>
struct MemoryLinear : public MemoryInterface<_Address_t> {
    using Address_t = _Address_t;

    using VectorType = std::vector<uint8_t>;

    Clock *const clock;
    const bool verbose;
    const Address_t size;

    VectorType memory_block;

    MemoryLinear(Clock *clock, Address_t size, VectorType init_content = {}, bool verbose = false)
        : clock(clock), verbose(verbose), size(size), memory_block(init_content) {
        memory_block.resize(size, 0);
    }

    uint8_t Load(Address_t address) const override {
        WaitForNextCycle();
        if (address >= size) {
            AccessLog(address, 0, false, true);
            throw std::runtime_error(
                fmt::format("Attempt to read out of bounds address {:04x} (size: {:04x})", address, size));
        }
        auto v = memory_block[address];
        AccessLog(address, v, false, false);
        return v;
    }

    void Store(Address_t address, uint8_t value) override {
        WaitForNextCycle();
        if (address >= size) {
            AccessLog(address, 0, true, true);
            throw std::runtime_error(
                fmt::format("Attempt to write out of bounds address {:04x} (size: {:04x})", address, size));
        }
        memory_block[address] = value;
        AccessLog(address, value, true, false);
    }

private:
    void AccessLog(Address_t address, uint8_t value, bool write, bool overflow = false) const {
        if (verbose) {
            std::cout << fmt::format("MEMBLOCK {:5} [{:04x}] {} {:02x} {}\n", (write ? "WRITE" : "READ"), address,
                                     (write ? "<-" : "->"), value, (overflow ? "OVERFLOW" : ""));
        }
    }

    void WaitForNextCycle() const {
        if (clock != nullptr) {
            clock->WaitForNextCycle();
        }
    }
};

using MemoryLinear16 = MemoryLinear<uint16_t>;

} // namespace emu::memory
