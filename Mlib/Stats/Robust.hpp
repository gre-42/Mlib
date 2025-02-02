#pragma once
#include <Mlib/Math/Math.hpp>
#include <Mlib/Stats/Sort.hpp>

namespace Mlib {

template <class TData>
TData median(const Array<TData>& a) {
    if (a.length() == 0) {
        return NAN;
    } else {
        Array<TData> af = sorted(a);
        return af(af.length() / 2);
    }
}

template <class TData>
TData median_inplace(Array<TData>& a) {
    if (a.length() == 0) {
        return NAN;
    } else {
        sort(a);
        return a(a.length() / 2);
    }
}

template <class TData>
TData nanmedian(const Array<TData>& a) {
    return median(a[!Mlib::isnan(a)]);
}

/**
 * Median absolute deviation.
 * Based on statsmodels.robust.scale.mad.
 * 0.6744897501960817 = norm.ppf(3 / 4)
 */
template <class TData>
TData mad(const Array<TData>& a, TData* median = nullptr, TData c = TData(0.674)) {
    TData me = ::Mlib::median(a);
    TData ma = ::Mlib::median(abs(a - me)) / c;
    if (median != nullptr) {
        *median = me;
    }
    return ma;
}

template <class TData>
Array<TData> robust_deviation(const Array<TData>& sample) {
    TData median;
    TData mad = ::Mlib::mad(sample, &median);
    return (sample - median) / mad;
}

}
