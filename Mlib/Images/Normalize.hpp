#pragma once
#include <Mlib/Math/Lerp_Array.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <algorithm>

namespace Mlib {

template <class TData>
TData clipped_element(const TData& v, const TData& low, const TData& high) {
    if (std::isnan(v)) {
        return v;
    } else {
        return std::clamp(v, low, high);
    }
}

template <class TData>
void clip(Array<TData>& a, const TData& low, const TData& high) {
    for (auto& v : a.flat_iterable()) {
        v = clipped_element(v, low, high);
    }
}

template <class TData>
Array<TData> clipped(const Array<TData>& a, const TData& low, const TData& high) {
    Array<TData> result = a.copy();
    clip(result, low, high);
    return result;
}

template <class TData>
void normalize_and_clip(
    Array<TData>& a,
    const TData& low, const TData& high,
    const TData& dlow = 0, const TData& dhigh = 1)
{
    lerp_array(dlow, dhigh, a, low, high);
    clip<TData>(a, dlow, dhigh);
}

template <class TData>
Array<TData> normalized_and_clipped(
    const Array<TData>& a,
    const TData& low, const TData& high,
    const TData& dlow = 0, const TData& dhigh = 1)
{
    Array<TData> result = a.copy();
    normalize_and_clip(result, low, high, dlow, dhigh);
    return result;
}

template <class TData>
void normalize_and_clip(Array<TData>& a) {
    Array<TData> va = a[Mlib::isfinite(a)];
    normalize_and_clip(a, min(va), max(va));
}

template <class TData>
Array<TData> normalized_and_clipped(const Array<TData>& a) {
    Array<TData> result = a.copy();
    normalize_and_clip(result);
    return result;
}

}
