#include "emu_core/byte_utils.hpp"
#include <cinttypes>
#include <fmt/format.h>
#include <limits>
#include <regex>
#include <stdexcept>
#include <typeinfo>

namespace emu {

namespace {

template <typename T>
T ParseIntegral(std::string_view text, int base) {
    size_t processed = 0;
    if (base == 0) {
        if (text.starts_with("$")) {
            text.remove_prefix(1);
            base = 16;
        }
    }
    auto raw = std::stol(std::string(text), &processed, base);
    using Limit = std::numeric_limits<T>;
    if (raw > Limit::max() || raw < Limit::min() || processed != text.size()) {
        throw std::runtime_error(fmt::format("Cannot parse '{}' into '{}'", text, typeid(T).name()));
    }
    return static_cast<T>(raw);
}

} // namespace

uint8_t ParseByte(std::string_view text, int base) {
    return ParseIntegral<uint8_t>(text, base);
}

uint16_t ParseWord(std::string_view text, int base) {
    return ParseIntegral<uint16_t>(text, base);
}

std::vector<uint8_t> ParsePackedIntegral(std::string_view text, std::optional<size_t> expected_size, int base) {
    auto word = ParseWord(text, base);
    if (word > 0xFF) {
        if (expected_size.value_or(2) != 2) {
            throw std::runtime_error(fmt::format("{} size exceeds byte size", text));
        }
        return ToBytes(word);
    }
    if (expected_size.value_or(1) == 1) {
        return ToBytes(static_cast<uint8_t>(word & 0xff));
    } else {
        return ToBytes(word);
    }
}

} // namespace emu
