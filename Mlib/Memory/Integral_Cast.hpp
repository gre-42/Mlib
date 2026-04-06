#pragma once
#include <concepts>
#include <limits>
#include <stdexcept>

namespace Mlib {

#ifdef __ANDROID__
template <typename TDest, typename TSource>
#else
template <std::integral TDest, std::integral TSource>
#endif
TDest integral_cast(TSource source) {
    if constexpr((double)std::numeric_limits<TSource>::max() > (double)std::numeric_limits<TDest>::max()) {
        if (source > (TSource)std::numeric_limits<TDest>::max()) {
            throw std::runtime_error("Value too large");
        }
    }
    if constexpr((double)std::numeric_limits<TSource>::min() < (double)std::numeric_limits<TDest>::min()) {
        if (source < (TSource)std::numeric_limits<TDest>::min()) {
            throw std::runtime_error("Value too small");
        }
    }
    return TDest(source);
}

}
