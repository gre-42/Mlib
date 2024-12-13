#include "Terrain_Uv.hpp"
#include <Mlib/Geometry/Material/Wrap_Mode.hpp>
#include <Mlib/Geometry/Mesh/Up_Axis.hpp>
#include <Mlib/Geometry/Texture/Uv_Shifter.hpp>
#include <Mlib/Scene_Precision.hpp>

using namespace Mlib;

template <class TPos, class TScale>
FixedArray<float, 3, 2> Mlib::terrain_uv(
    const FixedArray<TPos, 2>& a,
    const FixedArray<TPos, 2>& b,
    const FixedArray<TPos, 2>& c,
    TScale scale,
    TScale uv_scale,
    TScale period)
{
    TScale scale_total = uv_scale / scale;
    UvShifter3<TScale> uv_shifter{
        period,
        {funpack(a(0)) * scale_total, funpack(a(1)) * scale_total},
        {funpack(b(0)) * scale_total, funpack(b(1)) * scale_total},
        {funpack(c(0)) * scale_total, funpack(c(1)) * scale_total},
        {WrapMode::REPEAT, WrapMode::REPEAT}};
    return {
        uv_shifter.u0,
        uv_shifter.u1,
        uv_shifter.u2};
}

template <class TPos, class TScale>
FixedArray<float, 3, 2> Mlib::terrain_uv(
    const FixedArray<TPos, 3>& a,
    const FixedArray<TPos, 3>& b,
    const FixedArray<TPos, 3>& c,
    TScale scale,
    TScale uv_scale,
    TScale period,
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

template <class TPos, class TScale>
FixedArray<float, 4, 2> Mlib::terrain_uv(
    const FixedArray<TPos, 2>& a,
    const FixedArray<TPos, 2>& b,
    const FixedArray<TPos, 2>& c,
    const FixedArray<TPos, 2>& d,
    TScale scale,
    TScale uv_scale,
    TScale period)
{
    TScale scale_total = uv_scale / scale;
    UvShifter4<TScale> uv_shifter{
        period,
        {funpack(a(0)) * scale_total, funpack(a(1)) * scale_total},
        {funpack(b(0)) * scale_total, funpack(b(1)) * scale_total},
        {funpack(c(0)) * scale_total, funpack(c(1)) * scale_total},
        {funpack(d(0)) * scale_total, funpack(d(1)) * scale_total},
        {WrapMode::REPEAT, WrapMode::REPEAT}};
    return {
        uv_shifter.u0,
        uv_shifter.u1,
        uv_shifter.u2,
        uv_shifter.u3};
}

template FixedArray<float, 3, 2> Mlib::terrain_uv<float, float>(
    const FixedArray<float, 2>& a,
    const FixedArray<float, 2>& b,
    const FixedArray<float, 2>& c,
    float scale,
    float uv_scale,
    float period);
template FixedArray<float, 3, 2> Mlib::terrain_uv<CompressedScenePos, double>(
    const FixedArray<CompressedScenePos, 2>& a,
    const FixedArray<CompressedScenePos, 2>& b,
    const FixedArray<CompressedScenePos, 2>& c,
    double scale,
    double uv_scale,
    double period);

template FixedArray<float, 3, 2> Mlib::terrain_uv<float, float>(
    const FixedArray<float, 3>& a,
    const FixedArray<float, 3>& b,
    const FixedArray<float, 3>& c,
    float scale,
    float uv_scale,
    float period,
    UpAxis up_axis);
template FixedArray<float, 3, 2> Mlib::terrain_uv<CompressedScenePos, double>(
    const FixedArray<CompressedScenePos, 3>& a,
    const FixedArray<CompressedScenePos, 3>& b,
    const FixedArray<CompressedScenePos, 3>& c,
    double scale,
    double uv_scale,
    double period,
    UpAxis up_axis);
template FixedArray<float, 3, 2> Mlib::terrain_uv<double, double>(
    const FixedArray<double, 3>& a,
    const FixedArray<double, 3>& b,
    const FixedArray<double, 3>& c,
    double scale,
    double uv_scale,
    double period,
    UpAxis up_axis);

template FixedArray<float, 4, 2> Mlib::terrain_uv<float, float>(
    const FixedArray<float, 2>& a,
    const FixedArray<float, 2>& b,
    const FixedArray<float, 2>& c,
    const FixedArray<float, 2>& d,
    float scale,
    float uv_scale,
    float period);
template FixedArray<float, 4, 2> Mlib::terrain_uv<CompressedScenePos, double>(
    const FixedArray<CompressedScenePos, 2>& a,
    const FixedArray<CompressedScenePos, 2>& b,
    const FixedArray<CompressedScenePos, 2>& c,
    const FixedArray<CompressedScenePos, 2>& d,
    double scale,
    double uv_scale,
    double period);
template FixedArray<float, 4, 2> Mlib::terrain_uv<double, double>(
    const FixedArray<double, 2>& a,
    const FixedArray<double, 2>& b,
    const FixedArray<double, 2>& c,
    const FixedArray<double, 2>& d,
    double scale,
    double uv_scale,
    double period);
