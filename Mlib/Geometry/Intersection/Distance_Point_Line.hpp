#pragma once
#include <Mlib/Array/Array_Forward.hpp>

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

namespace Mlib {

/**
 * From: https://stackoverflow.com/a/2049593/2292832
 */
template <class TData>
TData distance_point_to_line(
    const FixedArray<TData, 2>& p,
    const FixedArray<TData, 2>& l0,
    const FixedArray<TData, 2>& l1,
    bool normalize)
{
    FixedArray<float, 2> n = {l1(1) - l0(1), l0(0) - l1(0)};
    if (normalize) {
        float len = std::sqrt(sum(squared(n)));
        if (len < 1e-12) {
            throw std::runtime_error("Could not calculate distance point to line, len=" + std::to_string(len));
        }
        n /= len;
    }
    return dot0d(p - l0, n);
}

template <class TData>
FixedArray<TData, 2> transform_to_line_coordinates(
    const FixedArray<TData, 2>& p,
    const FixedArray<TData, 2>& l0,
    const FixedArray<TData, 2>& l1)
{
    FixedArray<TData, 2> d = l1 - l0;
    TData dist = std::sqrt(sum(squared(d)));
    if (dist < 1e-12) {
        throw std::runtime_error("Could not transform to line coordinates, dist=" + std::to_string(dist));
    }
    d /= dist;
    FixedArray<TData, 2> n = {d(1), -d(0)};
    return FixedArray<TData, 2>{
        dot0d(p - l0, d) / dist,
        dot0d(p - l0, n)};
}

}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
