#pragma once
#include <Mlib/Math/Math.hpp>
#include <Mlib/Stats/Min_Max.hpp>

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
    for(auto& v : a.flat_iterable()) {
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
void normalize_and_clip(Array<TData>& a, const TData& low, const TData& high) {
    a -= low;
    a /= (high - low);
    clip<TData>(a, 0, 1);
}

template <class TData>
Array<TData> normalized_and_clipped(const Array<TData>& a, const TData& low, const TData& high) {
    Array<TData> result = a.copy();
    normalize_and_clip(result, low, high);
    return result;
}

template <class TData>
void normalize_and_clip(Array<TData>& a) {
    Array<TData> va = a[isfinite(a)];
    normalize_and_clip(a, min(va), max(va));
}

template <class TData>
Array<TData> normalized_and_clipped(const Array<TData>& a) {
    Array<TData> result = a.copy();
    normalize_and_clip(result);
    return result;
}

}
