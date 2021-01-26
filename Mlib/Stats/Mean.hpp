#pragma once
#include <Mlib/Get_Scalar.hpp>
#include <Mlib/Math/Math.hpp>

namespace Mlib {

template <class TDerived, class TData>
TData mean(const BaseDenseArray<TDerived, TData>& a) {
    typedef typename ScalarType<TData>::value_type ScalarType;
    return sum(a) / (ScalarType)a->nelements();
}

template <class TDerived, class TData>
TData nanmean(const BaseDenseArray<TDerived, TData>& a) {
    return mean((*a)[!Mlib::isnan(a)]);
}

}
