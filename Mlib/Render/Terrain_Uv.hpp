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

FixedArray<FixedArray<float, 2>, 3> terrain_uv(
    const FixedArray<double, 2>& a,
    const FixedArray<double, 2>& b,
    const FixedArray<double, 2>& c,
    double scale,
    double uv_scale,
    double period);

FixedArray<FixedArray<float, 2>, 3> terrain_uv(
    const FixedArray<double, 3>& a,
    const FixedArray<double, 3>& b,
    const FixedArray<double, 3>& c,
    double scale,
    double uv_scale,
    double period,
    UpAxis up_axis);

FixedArray<FixedArray<float, 2>, 4> terrain_uv(
    const FixedArray<double, 2>& a,
    const FixedArray<double, 2>& b,
    const FixedArray<double, 2>& c,
    const FixedArray<double, 2>& d,
    double scale,
    double uv_scale,
    double period);

}
