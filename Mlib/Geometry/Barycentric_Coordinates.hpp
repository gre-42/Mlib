#pragma once
#include <Mlib/Math/Fixed_Math.hpp>

namespace Mlib {

/**
 * From: https://gamedev.stackexchange.com/a/23745/140709
 * Compute barycentric coordinates (u, v, w) for
 * point p with respect to triangle (a, b, c)
 */
template <class TData, size_t tndim>
void barycentric(
    const FixedArray<TData, tndim>& p,
    const FixedArray<TData, tndim>& a,
    const FixedArray<TData, tndim>& b,
    const FixedArray<TData, tndim>& c,
    TData &u,
    TData &v,
    TData &w)
{
    FixedArray<TData, tndim> v0 = b - a;
    FixedArray<TData, tndim> v1 = c - a;
    FixedArray<TData, tndim> v2 = p - a;
    TData d00 = dot0d(v0, v0);
    TData d01 = dot0d(v0, v1);
    TData d11 = dot0d(v1, v1);
    TData d20 = dot0d(v2, v0);
    TData d21 = dot0d(v2, v1);
    TData denom = d00 * d11 - d01 * d01;
    v = (d11 * d20 - d01 * d21) / denom;
    w = (d00 * d21 - d01 * d20) / denom;
    u = 1 - v - w;
}

}
