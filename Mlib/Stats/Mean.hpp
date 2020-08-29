#pragma once
#include <Mlib/Math/Math.hpp>

namespace Mlib {

template <class TDerived, class TData>
TData mean(const BaseDenseArray<TDerived, TData>& a) {
    return sum(a) / TData(a->nelements());
}

template <class TDerived, class TData>
TData nanmean(const BaseDenseArray<TDerived, TData>& a) {
    return mean((*a)[!isnan(a)]);
}

}
