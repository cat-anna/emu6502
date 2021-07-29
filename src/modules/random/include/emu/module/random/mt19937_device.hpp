#pragma once

#include "emu_core/memory.hpp"
#include <cstdint>
#include <iostream>
#include <list>
#include <queue>
#include <random>

namespace emu::module::random {

class Mt19937Device : public Memory16 {
public:
    using IntType = uint32_t;
    using CrReg = uint8_t;
    using OutReg = uint8_t;

    enum class Register : Memory16::Address_t {
        kSeed0,
        kSeed1,
        kSeed2,
        kSeed3,
        kEntropy,
        kCR0,
    };

    static constexpr IntType kDefaultSeed = 0xDEADBEEF;
    static constexpr Memory16::Address_t kDeviceMemorySize =
        sizeof(IntType) + sizeof(OutReg) + sizeof(CrReg);

    Mt19937Device(IntType default_seed = kDefaultSeed,
                  std::ostream *verbose_output = nullptr);

    [[nodiscard]] uint8_t Load(Address_t address) const override;
    void Store(Address_t address, uint8_t value) override;

    [[nodiscard]] std::optional<uint8_t> DebugRead(Address_t address) const override;

private:
    std::ostream *const verbose_output;
    mutable std::uniform_int_distribution<> byte_distribution{0, 0xff};
    mutable std::mt19937 mt;
    IntType current_seed;
    uint8_t control_reg = 0;

    [[noreturn]] void Error(const std::string &msg) const {
        if (verbose_output != nullptr) {
            (*verbose_output) << msg << "\n";
        }
        throw std::runtime_error(msg);
    }
};

} // namespace emu::module::random
