#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace emu {

constexpr uint8_t operator"" _u8(unsigned long long n) {
    return static_cast<uint8_t>(n);
}

constexpr int8_t operator"" _s8(unsigned long long n) {
    return static_cast<int8_t>(n);
}

constexpr uint16_t operator"" _u16(unsigned long long n) {
    return static_cast<uint16_t>(n);
}
constexpr int16_t operator"" _416(unsigned long long n) {
    return static_cast<int16_t>(n);
}

uint8_t ParseByte(std::string_view text, int base = 0);
uint16_t ParseWord(std::string_view text, int base = 0);
std::vector<uint8_t>
ParsePackedIntegral(std::string_view text,
                    std::optional<size_t> expected_size = std::nullopt, int base = 0);

inline std::vector<uint8_t> ToBytes(std::monostate) {
    return {};
}

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

inline std::vector<uint8_t> ToBytes(std::string_view v) {
    return {v.begin(), v.end()};
}

} // namespace emu