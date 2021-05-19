#include "byte_utils.hpp"
#include <cinttypes>
#include <fmt/format.h>
#include <limits>
#include <regex>
#include <stdexcept>
#include <typeinfo>

namespace emu::emu6502::assembler {

namespace {

template <typename T>
T ParseIntegral(std::string_view text, int base) {
    size_t processed = 0;
    auto raw = std::stol(std::string(text), &processed, base);
    using Limit = std::numeric_limits<T>;
    if (raw > Limit::max() || raw < Limit::min() || processed != text.size()) {
        throw std::runtime_error(fmt::format("Cannot parse {} into {}", text, typeid(T).name()));
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

} // namespace emu::assembler6502
