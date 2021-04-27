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

}
