#pragma once
#include <Mlib/Array/Consteval_Workaround.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Memory/Integral_To_Float.hpp>
#include <type_traits>

namespace Mlib {

template <class TDerived, class TData>
TData mean(const BaseDenseArray<TDerived, TData>& a) {
    const auto& av = *a;  // Workaround for MSVC
    assert(av.nelements() > 0);
    return sum(a) / CW::nelements(av);
}

template <size_t axis, class TData, size_t... tsize>
auto mean(const FixedArray<TData, tsize...>& a) {
#ifdef _MSC_VER
    constexpr size_t n = FixedArray<TData, tsize...>::template static_shape<axis>();
#else
    constexpr size_t n = CW::static_shape<axis>(a);
#endif
    static_assert(n > 0);
    if constexpr (std::is_floating_point_v<TData>) {
        return sum<axis>(a) / integral_to_float<TData>(n);
    } else {
        return sum<axis>(a) / n;
    }
}

template <class TData>
Array<TData> mean(const Array<TData>& a, size_t axis) {
    assert(a.shape(axis) > 0);
    return sum(a, axis) / a.shape(axis);
}

template <class TDerived, class TData>
TData nanmean(const BaseDenseArray<TDerived, TData>& a) {
    return mean((*a)[!Mlib::isnan(a)]);
}

}
