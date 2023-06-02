#pragma once
#include <Mlib/Throw_Or_Abort.hpp>
#include <concepts>
#include <limits>

namespace Mlib {

#ifdef __ANDROID__
template <class TDest, class TSource>
struct FalseIntegralCast {
    static const bool false_ = false;
};

template <class TDest, class TSource>
inline TDest integral_cast(TSource source) {
    static_assert(FalseIntegralCast<TDest, TSource>::false_);
    THROW_OR_ABORT("Invalid call to integral_cast");
}

template <>
inline uint32_t integral_cast<uint32_t, int32_t>(int32_t source) {
    if (source < 0) {
        THROW_OR_ABORT("Value too small");
    }
    return (uint32_t)source;
}

template <>
inline int32_t integral_cast<int32_t, uint32_t>(uint32_t source) {
    if (source > (uint32_t)std::numeric_limits<int32_t>::max()) {
        THROW_OR_ABORT("Value too large");
    }
    return (int32_t)source;
}

template <>
inline uint64_t integral_cast<uint64_t, int64_t>(int64_t source) {
    if (source < 0) {
        THROW_OR_ABORT("Value too small");
    }
    return (uint64_t)source;
}

template <>
inline int64_t integral_cast<int64_t, uint64_t>(uint64_t source) {
    if (source > (uint64_t)std::numeric_limits<int64_t>::max()) {
        THROW_OR_ABORT("Value too large");
    }
    return (int64_t)source;
}

template <>
inline int32_t integral_cast<int32_t, uint64_t>(uint64_t source) {
    if (source > (uint64_t)std::numeric_limits<int32_t>::max()) {
        THROW_OR_ABORT("Value too large");
    }
    return (int32_t)source;
}

template <>
inline uint32_t integral_cast<uint32_t, int64_t>(int64_t source) {
    if (source > (int64_t)std::numeric_limits<uint32_t>::max()) {
        THROW_OR_ABORT("Value too large");
    }
    if (source < 0) {
        THROW_OR_ABORT("Value too small");
    }
    return (uint32_t)source;
}

template <>
inline uint32_t integral_cast<uint32_t, uint64_t>(uint64_t source) {
    if (source > (uint64_t)std::numeric_limits<uint32_t>::max()) {
        THROW_OR_ABORT("Value too large");
    }
    return (uint32_t)source;
}

template <>
inline uint64_t integral_cast<uint64_t, int32_t>(int32_t source) {
    if (source < 0) {
        THROW_OR_ABORT("Value too small");
    }
    return (uint64_t)source;
}
#else
template <std::integral TDest, std::integral TSource>
TDest integral_cast(TSource source) {
    if constexpr((double)std::numeric_limits<TSource>::max() > (double)std::numeric_limits<TDest>::max()) {
        if (source > (TSource)std::numeric_limits<TDest>::max()) {
            THROW_OR_ABORT("Value too large");
        }
    }
    if constexpr((double)std::numeric_limits<TSource>::min() < (double)std::numeric_limits<TDest>::min()) {
        if (source < (TSource)std::numeric_limits<TDest>::min()) {
            THROW_OR_ABORT("Value too small");
        }
    }
    return TDest(source);
}
#endif

}
