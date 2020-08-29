#pragma once
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Math/Fixed_Math.hpp>

namespace Mlib {

template <class TData>
inline FixedArray<TData, 2> scaled_line_normal(const FixedArray<FixedArray<TData, 2>, 2>& l)
{
    return FixedArray<float, 2>{l(1)(1) - l(0)(1), l(0)(0) - l(1)(0)};
}

template <class TData>
inline FixedArray<TData, 2> line_normal(const FixedArray<FixedArray<TData, 2>, 2>& l)
{
    FixedArray<TData, 2> res = scaled_line_normal(l);
    res /= std::sqrt(sum(squared(res)));
    return res;
}

}
