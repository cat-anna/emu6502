#pragma once

#include <cstdint>

namespace emu {

#ifdef DEBUG
constexpr bool kDebugBuild = true;
constexpr bool kOptimizedBuild = false;
#else
constexpr bool kOptimizedBuild = true;
constexpr bool kDebugBuild = false;
#endif

} // namespace emu
