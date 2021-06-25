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
struct MemoryBlock : public MemoryInterface<_Address_t> {
    using Address_t = _Address_t;

    using VectorType = std::vector<uint8_t>;

    Clock *const clock;
    const bool verbose;
    const MemoryMode mode;
    VectorType block;
    std::string name;

    MemoryBlock(Clock *clock, VectorType memory, MemoryMode mode = MemoryMode::kReadWrite,
                bool verbose = false, std::string name = "")
        : clock(clock), verbose(verbose), mode(mode), block(std::move(memory)),
          name(std::move(name)) {}

    uint8_t Load(Address_t address) const override {
        if (address > block.size()) {
            throw MemoryOutOfBoundAccessException(address, block.size(), "MemoryBlock");
        }
        WaitForNextCycle();
        auto v = block[address];
        AccessLog(address, v, false);
        return v;
    }

    void Store(Address_t address, uint8_t value) override {
        WaitForNextCycle();
        AccessLog(address, value, true);
        if (CanWrite(address)) {
            block[address] = value;
        }
    }

private:
    [[nodiscard]] bool CanWrite(Address_t address) {
        if (address > block.size()) {
            throw MemoryOutOfBoundAccessException(address, block.size(), "MemoryBlock");
        }

        switch (mode) {
        case MemoryMode::kReadOnly:
            return false;
        case MemoryMode::kThrowOnWrite:
            throw MemoryWriteAttemptException(address, block.size(), "MemoryBlock");
        case MemoryMode::kReadWrite:
            break;
        }
        return true;
    }

    void AccessLog(Address_t address, uint8_t value, bool write) const {
        if (verbose) {
            std::cout << fmt::format("MEM-BLOCK[{}] {:5} [{:04x}] {} {:02x}\n", name,
                                     (write ? "W" : "R"), address, (write ? "<-" : "->"),
                                     value);
        }
    }

    void WaitForNextCycle() const {
        if (clock != nullptr) {
            clock->WaitForNextCycle();
        }
    }
};

using MemoryBlock16 = MemoryBlock<uint16_t>;

} // namespace emu
