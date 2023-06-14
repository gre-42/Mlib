#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <vector>

namespace Mlib {

template <class TPos>
FixedArray<FixedArray<FixedArray<TPos, 3>, 3>, 7> convex_decomposition_terrain(
    const FixedArray<TPos, 3>& a,
    const FixedArray<TPos, 3>& b,
    const FixedArray<TPos, 3>& c,
    const FixedArray<float, 3>& shift_a,
    const FixedArray<float, 3>& shift_b,
    const FixedArray<float, 3>& shift_c);

}
