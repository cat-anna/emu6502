#pragma once

#include <fmt/format.h>
#include <string>

namespace emu {

template <typename iterable>
static std::string ToHex(const iterable &container) {
    std::string hex_table;
    for (auto v : container) {
        if (!hex_table.empty()) {
            hex_table += " ";
        }
        hex_table += fmt::format("{:02x}", v);
    }
    return hex_table;
}

template <typename iterable>
static std::string ToHexArray(const iterable &container) {
    std::string hex_table;
    for (auto v : container) {
        if (!hex_table.empty()) {
            hex_table += ",";
        }
        hex_table += fmt::format("0x{:02x}", v);
    }
    return hex_table;
}

} // namespace emu
