#pragma once
#include <Mlib/Math/Fixed_Math.hpp>

namespace Mlib {

template <class TData, size_t tndim>
bool ray_intersects_sphere(
    const FixedArray<TData, tndim>& ray_origin,
    const FixedArray<TData, tndim>& ray_direction,
    const FixedArray<TData, tndim>& sphere_center,
    TData radius_squared,
    TData* lambda0,
    TData* lambda1)
{
    auto R = ray_origin - sphere_center;
    auto b = 2 * dot0d(R, ray_direction);
    auto c = sum(squared(R)) - radius_squared;
    auto s = squared(b) - 4 * c;
    if (s < 0) {
        return false;
    }
    auto q = std::sqrt(s);
    if (lambda0 != nullptr) {
        *lambda0 = (-b + q) * TData(0.5);
    }
    if (lambda1 != nullptr) {
        *lambda1 = (-b - q) * TData(0.5);
    }
    return true;
}

}
