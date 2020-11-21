#pragma once
#include <Mlib/Array/Array_Forward.hpp>

#pragma GCC push_options
#pragma GCC optimize ("O3")

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
        n /= std::sqrt(sum(squared(n)));
    }
    return dot0d(p - l1, n);
}

}

#pragma GCC pop_options
