#pragma once

#include "clock.hpp"
#include "memory.hpp"
#include <algorithm>
#include <array>
#include <concepts>
#include <cstdint>
#include <fmt/format.h>
#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace emu {

template <std::unsigned_integral _Address_t>
struct SparseMemory : public MemoryInterface<_Address_t> {
    using Address_t = _Address_t;

    using MapType = std::unordered_map<Address_t, uint8_t>;
    using VectorType = std::vector<uint8_t>;

    Clock *const clock;
    const bool strict_access;
    const bool verbose;

    MapType memory_map;

    static uint8_t RandomByte() { return rand() & 0xFF; }

    SparseMemory(Clock *clock, bool strict_access = false, bool verbose = false)
        : clock(clock), strict_access(strict_access), verbose(verbose) {}

    uint8_t Load(Address_t address) const override {
        WaitForNextCycle();
        if (auto it = memory_map.find(address); it == memory_map.end()) {
            if (strict_access) {
                throw std::runtime_error(fmt::format("Attempt to read null address {:04x}", address));
            }
            auto v = RandomByte();
            AccessLog(address, v, false, true);
            return v;
        } else {
            auto v = it->second;
            AccessLog(address, v, false, false);
            return v;
        }
    }

    void Store(Address_t address, uint8_t value) override {
        WaitForNextCycle();
        bool is_null = memory_map.find(address) == memory_map.end();
        if (is_null && strict_access) {
            throw std::runtime_error(fmt::format("Attempt to write {:02x} to null address {:04x}", value, address));
        }
        AccessLog(address, value, true, is_null);
        memory_map[address] = value;
    }

    void WriteRange(Address_t addr, const VectorType &data) {
        for (Address_t pos = 0; pos < data.size(); ++pos) {
            memory_map[addr + pos] = data[pos];
        }
    }

    VectorType ReadRange(Address_t addr, size_t len) {
        VectorType r;
        for (Address_t pos = 0; pos < len; ++pos) {
            r.emplace_back(memory_map.at(addr + pos));
        }
        return r;
    }

    template <typename SparseIterable>
    void WriteSparse(const SparseIterable &data) {
        for (auto [addr, v] : data) {
            memory_map[addr] = v;
        }
    }

private:
    void AccessLog(Address_t address, uint8_t value, bool read, bool not_init = false) const {
        if (verbose) {
            std::cout << fmt::format("MEM {:5} [{:04x}] -> {:02x} {}\n", (read ? "READ" : "WRITE"), address, value,
                                     (not_init ? "NOT INITIALIZED" : ""));
        }
    }

    void WaitForNextCycle() const {
        if (clock != nullptr) {
            clock->WaitForNextCycle();
        }
    }
};

using SparseMemory16 = SparseMemory<uint16_t>;

} // namespace emu
