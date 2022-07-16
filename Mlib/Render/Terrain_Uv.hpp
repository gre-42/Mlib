#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

inline FixedArray<float, 2> terrain_uv(
    float x,
    float y,
    float scale,
    float uv_scale)
{
    return FixedArray<float, 2>{x, y} / scale * uv_scale;
}

inline FixedArray<float, 2> terrain_uv(
    const FixedArray<float, 3>& vertex,
    float scale,
    float uv_scale)
{
    return terrain_uv(vertex(0), vertex(1), scale, uv_scale);
}

FixedArray<FixedArray<float, 2>, 3> terrain_uv(
    const FixedArray<double, 2>& a,
    const FixedArray<double, 2>& b,
    const FixedArray<double, 2>& c,
    float scale,
    float uv_scale,
    float period);

FixedArray<FixedArray<float, 2>, 3> terrain_uv(
    const FixedArray<double, 3>& a,
    const FixedArray<double, 3>& b,
    const FixedArray<double, 3>& c,
    float scale,
    float uv_scale,
    float period);

FixedArray<FixedArray<float, 2>, 4> terrain_uv(
    const FixedArray<double, 2>& a,
    const FixedArray<double, 2>& b,
    const FixedArray<double, 2>& c,
    const FixedArray<double, 2>& d,
    float scale,
    float uv_scale,
    float period);

}
