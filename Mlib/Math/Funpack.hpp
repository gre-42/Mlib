#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Scaled_Integer.hpp>
#include <concepts>

namespace Mlib {

template <class TInt, std::intmax_t denominator>
inline auto funpack(const Array<ScaledInteger<TInt, denominator>>& a) {
    using I = intermediate_float<TInt>;
    return a.template casted<I>();
}

template <std::floating_point TFloat, size_t... tshape>
inline const auto& funpack(const Array<TFloat>& a) {
    return a;
}

template <class TInt, std::intmax_t denominator, size_t... tshape>
inline auto funpack(const FixedArray<ScaledInteger<TInt, denominator>, tshape...>& a) {
    using I = intermediate_float<TInt>;
    return a.template casted<I>();
}

template <std::floating_point TFloat, size_t... tshape>
inline const auto& funpack(const FixedArray<TFloat, tshape...>& a) {
    return a;
}

template <class TInt, std::intmax_t denominator>
inline auto funpack(const ScaledInteger<TInt, denominator>& a) {
    using I = intermediate_float<TInt>;
    return (I)a;
}

template <std::floating_point F>
inline auto funpack(const F& f) {
    return f;
}

template <std::integral I>
inline auto funpack(const I& i) {
    return i;
}

template <class T>
struct funpacks {
    static const T& func();
    using value_type = decltype(funpack(func()));
};

template <class T>
using funpack_t = funpacks<T>::value_type;

}
