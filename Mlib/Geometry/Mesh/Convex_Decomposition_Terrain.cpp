#include "Convex_Decomposition_Terrain.hpp"

using namespace Mlib;

template <class TPos>
FixedArray<FixedArray<FixedArray<TPos, 3>, 3>, 7> Mlib::convex_decomposition_terrain(
    const FixedArray<TPos, 3>& a,
    const FixedArray<TPos, 3>& b,
    const FixedArray<TPos, 3>& c,
    const FixedArray<TPos, 3>& shift)
{
    FixedArray<FixedArray<FixedArray<TPos, 3>, 3>, 7> result;
    FixedArray<TPos, 3> A = a + shift;
    FixedArray<TPos, 3> B = b + shift;
    FixedArray<TPos, 3> C = c + shift;
    result(0) = {a, b, c};

    result(1) = {a, A, b};
    result(2) = {A, B, b};

    result(3) = {b, B, c};
    result(4) = {B, C, c};

    result(5) = {c, C, a};
    result(6) = {C, A, a};
    return result;
}

template FixedArray<FixedArray<FixedArray<double, 3>, 3>, 7> Mlib::convex_decomposition_terrain<double>(
    const FixedArray<double, 3>& a,
    const FixedArray<double, 3>& b,
    const FixedArray<double, 3>& c,
    const FixedArray<double, 3>& shift);
