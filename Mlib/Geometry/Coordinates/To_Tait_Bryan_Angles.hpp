#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

template <class TData>
float z_to_yaw(const FixedArray<TData, 3>& z) {
    return float(-std::atan2(-z(0), z(2)));
}

template <class TData>
static float z_to_pitch(const FixedArray<TData, 3>& z) {
    return (float)std::atan2(-z(1), std::sqrt(squared(z(2)) + squared(z(0))));
}

}
