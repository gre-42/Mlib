#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Up_Axis.hpp>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
enum class UpAxis;

inline FixedArray<float, 2> terrain_uv(
    const FixedArray<float, 2>& pos,
    float scale,
    float uv_scale)
{
    return pos / scale * uv_scale;
}

inline FixedArray<float, 2> terrain_uv(
    const FixedArray<float, 3>& vertex,
    float scale,
    float uv_scale,
    UpAxis up_axis)
{
    return terrain_uv(non_up_axis(vertex, up_axis), scale, uv_scale);
}

template <class TPos>
FixedArray<FixedArray<float, 2>, 3> terrain_uv(
    const FixedArray<TPos, 2>& a,
    const FixedArray<TPos, 2>& b,
    const FixedArray<TPos, 2>& c,
    TPos scale,
    TPos uv_scale,
    TPos period);

template <class TPos>
FixedArray<FixedArray<float, 2>, 3> terrain_uv(
    const FixedArray<TPos, 3>& a,
    const FixedArray<TPos, 3>& b,
    const FixedArray<TPos, 3>& c,
    TPos scale,
    TPos uv_scale,
    TPos period,
    UpAxis up_axis);

template <class TPos>
FixedArray<FixedArray<float, 2>, 4> terrain_uv(
    const FixedArray<TPos, 2>& a,
    const FixedArray<TPos, 2>& b,
    const FixedArray<TPos, 2>& c,
    const FixedArray<TPos, 2>& d,
    TPos scale,
    TPos uv_scale,
    TPos period);

}
