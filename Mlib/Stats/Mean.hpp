#pragma once
#include <Mlib/Array/Consteval_Workaround.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Math.hpp>

namespace Mlib {

template <class TDerived, class TData>
TData mean(const BaseDenseArray<TDerived, TData>& a) {
    const auto& av = *a;  // Workaround for MSVC
    assert(av.nelements() > 0);
    return sum(a) / CW::nelements(av);
}


template <size_t axis, class TData, size_t... tsize>
auto mean(const FixedArray<TData, tsize...>& a) {
    constexpr size_t n = std::remove_reference_t<decltype(a)>::template static_shape<axis>();
    static_assert(n > 0);
    return sum<axis>(a) / n;
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
