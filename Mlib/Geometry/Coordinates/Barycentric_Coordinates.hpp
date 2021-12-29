#pragma once
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

/**
 * From: https://gamedev.stackexchange.com/a/23745/140709
 * Compute barycentric coordinates (u, v, w) for
 * point p with respect to triangle (a, b, c)
 */
void barycentric(
    const FixedArray<float, 2>& p,
    const FixedArray<float, 2>& a,
    const FixedArray<float, 2>& b,
    const FixedArray<float, 2>& c,
    float &u,
    float &v,
    float &w);

}
