#include "Terrain_Uv.hpp"
#include <Mlib/Render/Uv_Shifter.hpp>

using namespace Mlib;

FixedArray<FixedArray<float, 2>, 3> Mlib::terrain_uv(
    const FixedArray<double, 2>& a,
    const FixedArray<double, 2>& b,
    const FixedArray<double, 2>& c,
    float scale,
    float uv_scale,
    float period)
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
    float scale,
    float uv_scale,
    float period)
{
    return terrain_uv(
        FixedArray<double, 2>{a(0), a(1)},
        FixedArray<double, 2>{b(0), b(1)},
        FixedArray<double, 2>{c(0), c(1)},
        scale,
        uv_scale,
        period);
}

FixedArray<FixedArray<float, 2>, 4> Mlib::terrain_uv(
    const FixedArray<double, 2>& a,
    const FixedArray<double, 2>& b,
    const FixedArray<double, 2>& c,
    const FixedArray<double, 2>& d,
    float scale,
    float uv_scale,
    float period)
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
