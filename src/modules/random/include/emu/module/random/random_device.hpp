#pragma once

#include "emu_core/memory.hpp"
#include <cstdint>
#include <iostream>
#include <list>
#include <queue>
#include <random>

namespace emu::module::random {

class RandomDevice : public Memory16 {
public:
    using OutReg = uint8_t;

    static constexpr Memory16::Address_t kDeviceMemorySize = 1;

    RandomDevice(std::ostream *verbose_output = nullptr);

    [[nodiscard]] uint8_t Load(Address_t address) const override;
    void Store(Address_t address, uint8_t value) override;

    [[nodiscard]] std::optional<uint8_t> DebugRead(Address_t address) const override;

private:
    std::ostream *const verbose_output;
    mutable std::uniform_int_distribution<> byte_distribution{0, 0xff};
    mutable std::random_device rd;
};

} // namespace emu::module::random
