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
    const FixedArray<double, 2>& p,
    const FixedArray<double, 2>& a,
    const FixedArray<double, 2>& b,
    const FixedArray<double, 2>& c,
    double &u,
    double &v,
    double &w);

}
