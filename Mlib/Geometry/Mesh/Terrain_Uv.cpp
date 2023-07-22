#include "Terrain_Uv.hpp"
#include <Mlib/Geometry/Mesh/Up_Axis.hpp>
#include <Mlib/Geometry/Mesh/Uv_Shifter.hpp>

using namespace Mlib;

template <class TPos>
FixedArray<FixedArray<float, 2>, 3> Mlib::terrain_uv(
    const FixedArray<TPos, 2>& a,
    const FixedArray<TPos, 2>& b,
    const FixedArray<TPos, 2>& c,
    TPos scale,
    TPos uv_scale,
    TPos period)
{
    TPos scale_total = uv_scale / scale;
    UvShifter3<TPos> uv_shifter{
        period,
        {a(0) * scale_total, a(1) * scale_total},
        {b(0) * scale_total, b(1) * scale_total},
        {c(0) * scale_total, c(1) * scale_total}};
    return {
        uv_shifter.u0,
        uv_shifter.u1,
        uv_shifter.u2};
}

template <class TPos>
FixedArray<FixedArray<float, 2>, 3> Mlib::terrain_uv(
    const FixedArray<TPos, 3>& a,
    const FixedArray<TPos, 3>& b,
    const FixedArray<TPos, 3>& c,
    TPos scale,
    TPos uv_scale,
    TPos period,
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

template <class TPos>
FixedArray<FixedArray<float, 2>, 4> Mlib::terrain_uv(
    const FixedArray<TPos, 2>& a,
    const FixedArray<TPos, 2>& b,
    const FixedArray<TPos, 2>& c,
    const FixedArray<TPos, 2>& d,
    TPos scale,
    TPos uv_scale,
    TPos period)
{
    TPos scale_total = uv_scale / scale;
    UvShifter4<TPos> uv_shifter{
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

namespace Mlib {
    template FixedArray<FixedArray<float, 2>, 3> terrain_uv<float>(
        const FixedArray<float, 2>& a,
        const FixedArray<float, 2>& b,
        const FixedArray<float, 2>& c,
        float scale,
        float uv_scale,
        float period);
    template FixedArray<FixedArray<float, 2>, 3> terrain_uv<double>(
        const FixedArray<double, 2>& a,
        const FixedArray<double, 2>& b,
        const FixedArray<double, 2>& c,
        double scale,
        double uv_scale,
        double period);

    template FixedArray<FixedArray<float, 2>, 3> terrain_uv<float>(
        const FixedArray<float, 3>& a,
        const FixedArray<float, 3>& b,
        const FixedArray<float, 3>& c,
        float scale,
        float uv_scale,
        float period,
        UpAxis up_axis);
    template FixedArray<FixedArray<float, 2>, 3> terrain_uv<double>(
        const FixedArray<double, 3>& a,
        const FixedArray<double, 3>& b,
        const FixedArray<double, 3>& c,
        double scale,
        double uv_scale,
        double period,
        UpAxis up_axis);

    template FixedArray<FixedArray<float, 2>, 4> terrain_uv<float>(
        const FixedArray<float, 2>& a,
        const FixedArray<float, 2>& b,
        const FixedArray<float, 2>& c,
        const FixedArray<float, 2>& d,
        float scale,
        float uv_scale,
        float period);
    template FixedArray<FixedArray<float, 2>, 4> terrain_uv<double>(
        const FixedArray<double, 2>& a,
        const FixedArray<double, 2>& b,
        const FixedArray<double, 2>& c,
        const FixedArray<double, 2>& d,
        double scale,
        double uv_scale,
        double period);
}
