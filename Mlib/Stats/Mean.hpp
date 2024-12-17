#pragma once
#include <Mlib/Array/Consteval_Workaround.hpp>
#include <Mlib/Assert.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Memory/Integral_To_Float.hpp>
#include <Mlib/Type_Traits/Get_Scalar.hpp>
#include <Mlib/Type_Traits/Operand.hpp>

namespace Mlib {

template <class TData>
TData mean(const Array<TData>& a) {
    assert_true(a.nelements() > 0);
    return sum(a) / integral_to_float<scalar_type_t<TData>>(a.nelements());
}

template <class TData, size_t... tsize>
auto mean(const FixedArray<TData, tsize...>& a) {
    // Produces "function parameter 'a' with unknown value cannot be used in a constant expression".
    // constexpr size_t n = CW::nelements(a);
    constexpr size_t n = FixedArray<TData, tsize...>::nelements();
    static_assert(n > 0);
    return sum(a) / operand<scalar_type_t<TData>, n>;
}

template <size_t axis, class TData, size_t... tsize>
auto mean(const FixedArray<TData, tsize...>& a) {
    // Produces "function parameter 'a' with unknown value cannot be used in a constant expression".
    // constexpr size_t n = CW::static_shape<axis>(a);
    constexpr size_t n = FixedArray<TData, tsize...>::template static_shape<axis>();
    static_assert(n > 0);
    return sum<axis>(a) / operand<TData, n>;
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
