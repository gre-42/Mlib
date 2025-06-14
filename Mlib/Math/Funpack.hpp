#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Math/Fixed_Point_Number.hpp>
#include <concepts>

namespace Mlib {

template <class TInt, std::intmax_t denominator>
constexpr inline auto funpack(const Array<FixedPointNumber<TInt, denominator>>& a) {
    using I = intermediate_float<TInt>;
    return a.template casted<I>();
}

template <std::floating_point TFloat, size_t... tshape>
constexpr inline const auto& funpack(const Array<TFloat>& a) {
    return a;
}

template <class TInt, std::intmax_t denominator, size_t... tshape>
constexpr inline auto funpack(const FixedArray<FixedPointNumber<TInt, denominator>, tshape...>& a) {
    using I = intermediate_float<TInt>;
    return a.template casted<I>();
}

template <std::floating_point TFloat, size_t... tshape>
constexpr inline const auto& funpack(const FixedArray<TFloat, tshape...>& a) {
    return a;
}

template <class TInt, std::intmax_t denominator>
constexpr inline auto funpack(const FixedPointNumber<TInt, denominator>& a) {
    using I = intermediate_float<TInt>;
    return (I)a;
}

template <std::floating_point F>
constexpr inline auto funpack(const F& f) {
    return f;
}

template <std::integral I>
constexpr inline auto funpack(const I& i) {
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
