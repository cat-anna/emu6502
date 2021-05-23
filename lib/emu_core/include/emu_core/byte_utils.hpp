#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace emu {

uint8_t ParseByte(std::string_view text, int base = 0);
uint16_t ParseWord(std::string_view text, int base = 0);

inline std::vector<uint8_t> ToBytes(uint8_t v) {
    return {v};
}

inline std::vector<uint8_t> ToBytes(int8_t v) {
    return {static_cast<uint8_t>(v)};
}

inline std::vector<uint8_t> ToBytes(uint16_t v) {
    return {
        static_cast<uint8_t>(v & 0xFF),
        static_cast<uint8_t>(v >> 8),
    };
}

} // namespace emu