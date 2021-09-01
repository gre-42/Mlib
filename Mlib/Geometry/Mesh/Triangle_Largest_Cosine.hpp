#pragma once
#include <Mlib/Math/Fixed_Math.hpp>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

template <class TData, size_t tndim>
TData triangle_largest_cosine(const FixedArray<FixedArray<TData, tndim>, 3>& tri)
{
    TData lengths[]{
        sum(squared(tri(1) - tri(2))),
        sum(squared(tri(2) - tri(0))),
        sum(squared(tri(0) - tri(1)))};
    size_t i_best = 0;
    for (size_t i = 1; i < 3; ++i) {
        if (lengths[i] < lengths[i_best]) {
            i_best = i;
        }
    }
    size_t i_n = (i_best + 2) % 3;
    size_t i_p = (i_best + 1) % 3;
    return dot0d(
        tri(i_p) - tri(i_best),
        tri(i_n) - tri(i_best)) /
        std::sqrt(lengths[i_n] * lengths[i_p]);
}

}
