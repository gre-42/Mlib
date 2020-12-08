#pragma once
#include <Mlib/Math/Fixed_Math.hpp>

namespace Mlib {

template <class TData>
bool triangle_is_right_handed(
    const FixedArray<TData, 2>& a,
    const FixedArray<TData, 2>& b,
    const FixedArray<TData, 2>& c)
{
    FixedArray<TData, 2> e0 = b - a;
    FixedArray<TData, 2> e1 = c - b;
    FixedArray<TData, 2> n0{e0(1), -e0(0)};
    return dot0d(n0, e1) < 0;
}

}
