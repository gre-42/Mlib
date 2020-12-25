#pragma once
#include <Mlib/Math/Math.hpp>

namespace Mlib {

template <class TDerived, class TData>
auto maximum(const BaseDenseArray<TDerived, TData>& a, const BaseDenseArray<TDerived, TData>& b) {
    return a->array_array_binop(*b, [](const TData& x, const TData& y) { return std::max(x, y); });
}

template <class TDerived, class TData>
auto maximum(const BaseDenseArray<TDerived, TData>& a, const TData& b) {
    return a->applied([&b](const TData& x) { return std::max(x, b); });
}

template <class TDerived, class TData>
auto maximum(const TData& a, const BaseDenseArray<TDerived, TData>& b) {
    return b->applied([&a](const TData& x) { return std::max(a, x); });
}

template <class TDerived, class TData>
auto minimum(const BaseDenseArray<TDerived, TData>& a, const BaseDenseArray<TDerived, TData>& b) {
    return a->array_array_binop(*b, [](const TData& x, const TData& y) { return std::min(x, y); });
}

template <class TDerived, class TData>
auto minimum(const BaseDenseArray<TDerived, TData>& a, const TData& b) {
    return a->applied([&b](const TData& x) { return std::min(x, b); });
}

template <class TDerived, class TData>
auto minimum(const TData& a, const BaseDenseArray<TDerived, TData>& b) {
    return b->applied([&a](const TData& x) { return std::min(a, x); });
}

template <class TData>
Array<TData> max(const Array<TData>& x, size_t axis) {
    return x.apply_over_axis(axis, ApplyOverAxisType::REDUCE,
        [&](size_t i, size_t k, const Array<TData>& xf, Array<TData>& rf)
        {
            rf(i, k) = -INFINITY;
            for (size_t h = 0; h < x.shape(axis); ++h) {
                rf(i, k) = std::max(rf(i, k), xf(i, h, k));
            }
        });
}

template <class TData>
Array<TData> min(const Array<TData>& x, size_t axis) {
    return x.apply_over_axis(axis, ApplyOverAxisType::REDUCE,
        [&](size_t i, size_t k, const Array<TData>& xf, Array<TData>& rf)
        {
            rf(i, k) = -INFINITY;
            for (size_t h = 0; h < x.shape(axis); ++h) {
                rf(i, k) = std::min(rf(i, k), xf(i, h, k));
            }
        });
}

template <class TDerived, class TData>
TData max(const BaseDenseArray<TDerived, TData>& a) {
    auto f = a->flat_iterable();
    if (f.begin() == f.end()) {
        throw std::runtime_error("Cannot determine maximum of array of length 0");
    }
    TData result = *f.begin();
    for (auto it = f.begin() + 1; it != f.end(); ++it) {
        result = std::max(result, *it);
    }
    return result;
}

template <class TDerived, class TData>
TData min(const BaseDenseArray<TDerived, TData>& a) {
    auto f = a->flat_iterable();
    if (f.begin() == f.end()) {
        throw std::runtime_error("Cannot determine minimum of array of length 0");
    }
    TData result = *f.begin();
    for (auto it = f.begin() + 1; it != f.end(); ++it) {
        result = std::min(result, *it);
    }
    return result;
}

template <class TData>
TData nanmax(const Array<TData>& a) {
    return max(a[!isnan(a)]);
}

template <class TData>
TData nanmin(const Array<TData>& a) {
    return min(a[!isnan(a)]);
}

}
