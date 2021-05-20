#pragma once

#include <concepts>
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

template <std::unsigned_integral Address_t>
std::string SparseHexDump(const std::unordered_map<Address_t, uint8_t> &sparse_map, std::string_view line_prefix = "") {
    if (sparse_map.empty()) {
        return "";
    }

    auto [min_it, max_it] =
        std::minmax_element(sparse_map.begin(), sparse_map.end(), [](auto &a, auto &b) { return a.first < b.first; });
    size_t min = min_it->first & 0xFFF0;
    size_t max = max_it->first | 0x000F;

    std::string r;
    for (size_t pos = min; pos < max; pos += 0x10) {
        std::string hexes;
        bool any_byte = false;
        for (size_t off = 0; off <= 0xF; ++off) {
            auto it = sparse_map.find(static_cast<Address_t>(pos + off));
            if (it == sparse_map.end()) {
                hexes += " --";
            } else {
                any_byte = true;
                hexes += fmt::format(" {:02x}", it->second);
            }
        }
        if (any_byte) {
            r += fmt::format("{}{:04x} |{}\n", line_prefix, pos, hexes);
        }
    }

    return r;
}

} // namespace emu
