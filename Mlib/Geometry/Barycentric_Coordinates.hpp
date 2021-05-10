#pragma once
#include <Mlib/Geometry/Mesh/Triangle_Exception.hpp>
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
    FixedArray<double, tndim> v0 = (b - a).casted<double>();
    FixedArray<double, tndim> v1 = (c - a).casted<double>();
    FixedArray<double, tndim> v2 = (p - a).casted<double>();
    double d00 = dot0d(v0, v0);
    double d01 = dot0d(v0, v1);
    double d11 = dot0d(v1, v1);
    double d20 = dot0d(v2, v0);
    double d21 = dot0d(v2, v1);
    double denom = d00 * d11 - d01 * d01;
    // FixedArray<TData, tndim, tndim> M = dot2d(v0.columns_as_1D(), v1.rows_as_1D());
    // TData denom = dot0d(v0, dot1d(M - M.T(), v1));
    if (std::abs(denom) < 1e-14) {
        throw TriangleException(a, b, c, "barycentric coordinates encountered zero denominator");
    }
    v = (TData)((d11 * d20 - d01 * d21) / denom);
    w = (TData)((d00 * d21 - d01 * d20) / denom);
    u = (TData)(1 - v - w);
}

}
