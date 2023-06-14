#include "Convex_Decomposition_Terrain.hpp"
#include <Mlib/Geometry/Mesh/Triangle_Exception.hpp>
#include <Mlib/Math/Fixed_Math.hpp>

using namespace Mlib;

template <class TPos>
FixedArray<FixedArray<FixedArray<TPos, 3>, 3>, 8> Mlib::convex_decomposition_terrain(
    const FixedArray<TPos, 3>& a,
    const FixedArray<TPos, 3>& b,
    const FixedArray<TPos, 3>& c,
    const FixedArray<float, 3>& shift_a,
    const FixedArray<float, 3>& shift_b,
    const FixedArray<float, 3>& shift_c)
{
    FixedArray<FixedArray<FixedArray<TPos, 3>, 3>, 8> result;
    FixedArray<TPos, 3> A = a + shift_a.casted<TPos>();
    FixedArray<TPos, 3> B = b + shift_b.casted<TPos>();
    FixedArray<TPos, 3> C = c + shift_c.casted<TPos>();
    result(0) = {a, b, c};

    result(1) = {a, A, b};
    result(2) = {A, B, b};

    result(3) = {b, B, c};
    result(4) = {B, C, c};

    result(5) = {c, C, a};
    result(6) = {C, A, a};

    // Closing the box so collisions with the bottom do not cause a program
    // crash due to non-convexity.
    result(7) = {B, A, C};

    if ((dot0d(b - a, B - A) <= 0) ||
        (dot0d(c - b, C - B) <= 0) ||
        (dot0d(a - c, A - C) <= 0))
    {
        throw TriangleException<TPos>{a, b, c, "convex_decomposition_terrain: consistency-check failed"};
    }
    return result;
}

template FixedArray<FixedArray<FixedArray<double, 3>, 3>, 8> Mlib::convex_decomposition_terrain<double>(
    const FixedArray<double, 3>& a,
    const FixedArray<double, 3>& b,
    const FixedArray<double, 3>& c,
    const FixedArray<float, 3>& shift_a,
    const FixedArray<float, 3>& shift_b,
    const FixedArray<float, 3>& shift_c);
