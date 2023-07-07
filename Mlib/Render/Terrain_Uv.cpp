#include "Terrain_Uv.hpp"
#include <Mlib/Render/Uv_Shifter.hpp>

using namespace Mlib;

FixedArray<FixedArray<float, 2>, 3> Mlib::terrain_uv(
    const FixedArray<double, 2>& a,
    const FixedArray<double, 2>& b,
    const FixedArray<double, 2>& c,
    double scale,
    double uv_scale,
    double period)
{
    double scale_total = uv_scale / scale;
    UvShifter3 uv_shifter{
        period,
        {a(0) * scale_total, a(1) * scale_total},
        {b(0) * scale_total, b(1) * scale_total},
        {c(0) * scale_total, c(1) * scale_total}};
    return {
        uv_shifter.u0,
        uv_shifter.u1,
        uv_shifter.u2};
}

FixedArray<FixedArray<float, 2>, 3> Mlib::terrain_uv(
    const FixedArray<double, 3>& a,
    const FixedArray<double, 3>& b,
    const FixedArray<double, 3>& c,
    double scale,
    double uv_scale,
    double period,
    UpAxis up_axis)
{
    return terrain_uv(
        non_up_axis(a, up_axis),
        non_up_axis(b, up_axis),
        non_up_axis(c, up_axis),
        scale,
        uv_scale,
        period);
}

FixedArray<FixedArray<float, 2>, 4> Mlib::terrain_uv(
    const FixedArray<double, 2>& a,
    const FixedArray<double, 2>& b,
    const FixedArray<double, 2>& c,
    const FixedArray<double, 2>& d,
    double scale,
    double uv_scale,
    double period)
{
    double scale_total = uv_scale / scale;
    UvShifter4 uv_shifter{
        period,
        {a(0) * scale_total, a(1) * scale_total},
        {b(0) * scale_total, b(1) * scale_total},
        {c(0) * scale_total, c(1) * scale_total},
        {d(0) * scale_total, d(1) * scale_total}};
    return {
        uv_shifter.u0,
        uv_shifter.u1,
        uv_shifter.u2,
        uv_shifter.u3};
}
