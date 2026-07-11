#pragma once
#include <Mlib/Math/Lerp.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>

namespace Mlib {

template <class TData>
Array<TData> interpolate(const Array<TData>& a, const Array<TData>& y) {
    return a.applied([&](const TData& x){
        if (std::isnan(x)) {
            return NAN;
        }
        size_t left_id = integral_cast<size_t>(std::lround(std::floor(x)));
        size_t right_id = left_id + 1;
        if (right_id == y.length() && x <= TData(y.length() - 1)) {
            return y(left_id);
        } else if (left_id < y.length() && right_id < y.length()) {
            TData h = x - (TData)left_id;
            return lerp(y(left_id), y(right_id), h);
        } else {
            return NAN;
        }
    });
}

}
