#pragma once
#include <Mlib/Math/Math.hpp>
#include <Mlib/Stats/Arange.hpp>

namespace Mlib {

enum class SortingDirection {
    ascending,
    descending
};

template <class TData>
Array<size_t> argsort(const Array<TData>& s, SortingDirection sd = SortingDirection::ascending) {
    Array<size_t> ids = arange<size_t>(s.length());
    if (s.length() != 0) {
        if (sd == SortingDirection::ascending) {
            std::sort(&ids(0), &ids(0) + ids.length(), [&](size_t a, size_t b){return s(a) < s(b);});
        } else {
            std::sort(&ids(0), &ids(0) + ids.length(), [&](size_t a, size_t b){return s(a) > s(b);});
        }
    }
    return ids;
}

template <class TData>
void sort(Array<TData>& a) {
    assert(a.ndim() == 1);
    if (a.length() != 0) {
        TData* begin = &a(0);
        TData* end = begin + a.length();
        std::sort(begin, end);
    }
}

template <class TData>
Array<TData> sorted(const Array<TData>& a) {
    Array<TData> af{a.copy()};
    sort(af);
    return af;
}

template <class TData>
size_t argmin(const Array<TData>& a) {
    assert(a.ndim() == 1);
    size_t best_id = SIZE_MAX;
    TData best_value = INFINITY;
    for (size_t i = 0; i < a.length(); ++i) {
        // arithmetic exception in release-mode without std::isnan
        if (!std::isnan(a(i)) && a(i) < best_value) {
            best_id = i;
            best_value = a(i);
        }
    }
    return best_id;
}

template <class TData>
Array<size_t> argmin(const Array<TData>& a, size_t axis) {
    return a.template apply_over_axis<size_t>(axis, ApplyOverAxisType::REDUCE,
        [&](size_t i, size_t k, const Array<float>& xf, Array<size_t>& rf){
            size_t best_id = SIZE_MAX;
            TData best_value = INFINITY;
            for (size_t h = 0; h < a.shape(axis); ++h) {
                // arithmetic exception in release-mode without std::isnan
                if (!std::isnan(xf(i, h, k)) && xf(i, h, k) < best_value) {
                    best_id = h;
                    best_value = xf(i, h, k);
                }
            }
            rf(i, k) = best_id;
        });
}

}
