#pragma once
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Math/Fixed_Math.hpp>

namespace Mlib {

template <class TData>
std::optional<FixedArray<TData, 3, 3>> gl_lookat_relative(
    const FixedArray<TData, 3>& dz,
    const FixedArray<TData, 3>& dy0 = { 0.f, 1.f, 0.f })
{
    auto dx = cross(dy0, -dz);
    auto dx_len2 = sum(squared(dx));
    if (dx_len2 < 1e-12) {
        return std::nullopt;
    }
    dx /= std::sqrt(dx_len2);
    FixedArray<TData, 3> dy = cross(-dz, dx);
    return FixedArray<TData, 3, 3>::init(
        dx(0), dy(0), -dz(0),
        dx(1), dy(1), -dz(1),
        dx(2), dy(2), -dz(2));
}

template <class TData>
std::optional<FixedArray<TData, 3, 3>> gl_lookat_absolute(
    const FixedArray<TData, 3>& camera_pos,
    const FixedArray<TData, 3>& object_pos,
    const FixedArray<TData, 3>& dy0 = { 0.f, 1.f, 0.f })
{
    auto dz = object_pos - camera_pos;
    auto dz_len2 = sum(squared(dz));
    if (dz_len2 < 1e-12) {
        return std::nullopt;
    }
    return gl_lookat_relative(dz / std::sqrt(dz_len2), dy0);
}

}
