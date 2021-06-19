#pragma once

#include <concepts>
#include <cstdint>
#include <string_view>

namespace emu {

template <std::unsigned_integral T>
constexpr T MakeBitMask(T size) {
    T t{0};
    for (T i = 0; i < size; ++i) {
        t = (t << 1) | 1;
    }
    return t;
}

template <std::unsigned_integral T = uint8_t>
struct BitField {
    using Field_t = T;

    constexpr BitField(Field_t o, Field_t l) : offset(o), mask(MakeBitMask(l)) {}

    Field_t offset;
    Field_t mask;

    Field_t Get(Field_t value) const { return (value >> offset) & mask; }
    void Set(Field_t &output, Field_t value) const {
        output = (output & ~(mask << offset)) | ((value & mask) << offset);
    }
};

} // namespace emu
