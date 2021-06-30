#pragma once

#include <algorithm>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace emu {

template <typename F>
inline bool VerifyString(std::string_view s, F f) {
    for (const auto &i : s) {
        if (!f(i)) {
            return false;
        }
    }
    return true;
}

inline std::string ToUpper(const std::string &s) {
    std::string r;
    std::transform(s.begin(), s.end(), std::back_inserter(r),
                   [](char c) -> char { return static_cast<char>(::toupper(c)); });
    return r;
}

} // namespace emu
