#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Stats/Robust.hpp>
#include <algorithm>

namespace Mlib {

template <class TData>
Array<TData> median_filter_2d(
    const Array<TData>& im,
    size_t window_size,
    size_t minelements = 1,
    TData boundary_value = NAN)
{
    assert(im.ndim() == 2);
    assert(minelements > 0);
    std::vector<TData> values((2 * window_size + 1) * (2 * window_size + 1));
    Array<TData> result = full(im.shape(), boundary_value);
    if (any(im.shape() < window_size)) {
        return result;
    }
    for (size_t r = window_size; r < im.shape(0) - window_size; ++r) {
        for (size_t c = window_size; c < im.shape(1) - window_size; ++c) {
            size_t nvals = 0;
            for (size_t rr = 0; rr < 2 * window_size + 1; ++rr) {
                for (size_t cc = 0; cc < 2 * window_size + 1; ++cc) {
                    const TData& v = im(r + rr - window_size, c + cc - window_size);
                    if (!std::isnan(v)) {
                        values[nvals++] = v;
                    }
                }
            }
            if (nvals >= minelements) {
                std::sort(values, values + nvals);
                result(r, c) = values[nvals / 2];
            } else {
                result(r, c) = boundary_value;
            }
        }
    }
    return result;
}

}
