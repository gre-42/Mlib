#pragma once
#include <Mlib/Stats/Mean.hpp>

namespace Mlib {

template <class TDerivedA, class TDerivedB, class TData>
TData correlation(
    const BaseDenseArray<TDerivedA, TData>& a,
    const BaseDenseArray<TDerivedB, TData>& b,
    const TData& variance_threshold = 0)
{
    auto ma = *a - mean(*a);
    auto mb = *b - mean(*b);
    TData va = sum(squared(ma)) / (a->length() - 1);
    TData vb = sum(squared(mb)) / (b->length() - 1);
    if (std::abs(va) < variance_threshold || std::abs(vb) < variance_threshold) {
        return TData(0);
    }
    return dot0d(ma, mb) / std::sqrt(va * vb);
}

template <class TDerivedA, class TDerivedB, class TData>
TData normalized_difference(
    const BaseDenseArray<TDerivedA, TData>& a,
    const BaseDenseArray<TDerivedB, TData>& b,
    const TData& variance_threshold = 0)
{
    TData da = sum(squared(*a));
    TData db = sum(squared(*b));
    if (std::abs(da) < variance_threshold || std::abs(db) < variance_threshold) {
        return TData(0);
    }
    return sum(squared(*a - *b)) / std::sqrt(da * db);
}

}
