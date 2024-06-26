#pragma once
#include <Mlib/Array/Consteval_Workaround.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Type_Traits/Get_Scalar.hpp>

namespace Mlib {

template <class TDerived, class TData>
TData mean(const BaseDenseArray<TDerived, TData>& a) {
    const auto& av = *a;  // Workaround for MSVC
    assert(av.nelements() > 0);
    using ScalarType = scalar_type_t<TData>;
    return sum(a) / (ScalarType)CW::nelements(av);
}

template <class TData>
Array<TData> mean(const Array<TData>& a, size_t axis) {
    assert(a.shape(axis) > 0);
    using ScalarType = scalar_type_t<TData>;
    return sum(a, axis) / (ScalarType)a.shape(axis);
}

template <class TDerived, class TData>
TData nanmean(const BaseDenseArray<TDerived, TData>& a) {
    return mean((*a)[!Mlib::isnan(a)]);
}

}
