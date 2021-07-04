#pragma once

#include <algorithm>
#include <set>
#include <string>

namespace emu {

template <typename iterable>
static std::set<typename iterable::key_type> SortedKeys(const iterable &container) {
    std::set<typename iterable::key_type> keys;
    for (auto v : container) {
        keys.insert(v.first);
    }
    return keys;
}

} // namespace emu