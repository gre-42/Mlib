#pragma once
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Math/Lerp_Array.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Memory/Integral_To_Float.hpp>
#include <concepts>

namespace Mlib {

template <std::floating_point TDest, std::integral TInt>
Array<TDest> normalized_integral(
    const Array<TInt>& a,
    const TDest& dlow = 0, const TDest& dhigh = 1)
{
    auto tmp = normalized_and_clipped(
        a.template casted<double>(),
        integral_to_float<double>(std::numeric_limits<TInt>::min()),
        integral_to_float<double>(std::numeric_limits<TInt>::max()),
        (double)dlow,
        (double)dhigh);
    if constexpr (std::is_same_v<TDest, double>) {
        return tmp;
    } else {
        return tmp.template casted<TDest>();
    }
}

template <std::integral TDest, std::floating_point TFloat>
Array<TDest> denormalized_integral(const Array<TFloat>& a) {
    auto min = integral_to_float<double>(std::numeric_limits<TDest>::min());
    auto max = integral_to_float<double>(std::numeric_limits<TDest>::max());
    if constexpr (std::is_same_v<TDest, double>) {
        return ::Mlib::round(clipped(lerp_array(min, max, a), min, max)).template casted<TDest>();
    } else {
        return ::Mlib::round(clipped(lerp_array(min, max, a.template casted<double>()), min, max)).template casted<TDest>();
    }
}

}
