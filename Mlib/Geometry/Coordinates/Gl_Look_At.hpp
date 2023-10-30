#pragma once
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Math/Fixed_Math.hpp>

namespace Mlib {

template <class TData>
FixedArray<TData, 3, 3> gl_lookat_relative(
    const FixedArray<TData, 3>& dz,
    const FixedArray<TData, 3>& dy0 = { 0.f, 1.f, 0.f })
{
    auto dx = cross(dy0, -dz);
    dx /= std::sqrt(sum(squared(dx)));
    FixedArray<TData, 3> dy = cross(-dz, dx);
    return FixedArray<TData, 3, 3>::init(
        dx(0), dy(0), -dz(0),
        dx(1), dy(1), -dz(1),
        dx(2), dy(2), -dz(2));
}

template <class TData>
FixedArray<TData, 3, 3> gl_lookat_absolute(
    const FixedArray<TData, 3>& camera_pos,
    const FixedArray<TData, 3>& object_pos,
    const FixedArray<TData, 3>& dy0 = { 0.f, 1.f, 0.f })
{
    auto dz = object_pos - camera_pos;
    return gl_lookat_relative(dz / std::sqrt(sum(squared(dz))), dy0);
}

}
