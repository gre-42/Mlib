#pragma once
#include <Mlib/Math/Fixed_Point_Number.hpp>
#include <Mlib/Physics/Units.hpp>
#include <concepts>
#include <cstddef>
#include <limits>

namespace Mlib {

template <std::signed_integral T>
constexpr size_t w_shift(float max) {
    size_t result = 0;
    // max<T>() / (1 << result) >= max => max * (1 << result) <= max<T>()
    while (max * float(1 << result) < std::numeric_limits<T>::max()) {
        ++result;
    }
    return result;
}

static constexpr const std::intmax_t SCENE_W_8_DENOMINATOR = (1 << w_shift<int8_t>(1.f * rps));
using CompressedSceneW8 = FixedPointNumber<int8_t, SCENE_W_8_DENOMINATOR>;

static constexpr const std::intmax_t SCENE_V_8_DENOMINATOR = (1 << w_shift<int8_t>(200.f * kph));
using CompressedSceneV8 = FixedPointNumber<int8_t, SCENE_V_8_DENOMINATOR>;

}
