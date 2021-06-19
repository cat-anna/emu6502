#pragma once

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

} // namespace emu
