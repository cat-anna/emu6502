#pragma once

#include <concepts>
#include <cstdint>
#include <vector>

namespace emu {

template <std::unsigned_integral _Address_t>
class MemoryInterface {
public:
    using Address_t = _Address_t;

    virtual ~MemoryInterface() = default;

    virtual uint8_t Load(Address_t address) const = 0;
    virtual void Store(Address_t address, uint8_t value) = 0;
};

using Memory16 = MemoryInterface<uint16_t>;

} // namespace emu
