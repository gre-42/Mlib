#pragma once
#include <Mlib/Math/Math.hpp>
#include <Mlib/Type_Traits/Get_Scalar.hpp>

namespace Mlib {

template <class TDerived, class TData>
TData mean(const BaseDenseArray<TDerived, TData>& a) {
    assert(a->nelements() > 0);
    typedef typename ScalarType<TData>::value_type ScalarType;
    return sum(a) / (ScalarType)a->nelements();
}

template <class TData>
Array<TData> mean(const Array<TData>& a, size_t axis) {
    assert(a.shape(axis) > 0);
    typedef typename ScalarType<TData>::value_type ScalarType;
    return sum(a, axis) / (ScalarType)a.shape(axis);
}

template <class TDerived, class TData>
TData nanmean(const BaseDenseArray<TDerived, TData>& a) {
    return mean((*a)[!Mlib::isnan(a)]);
}

}
