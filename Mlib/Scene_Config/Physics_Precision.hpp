#pragma once
#include <Mlib/Math/Fixed_Point_Number.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <concepts>
#include <cstddef>
#include <limits>

namespace Mlib {

// Absolute + lowres
static constexpr const std::intmax_t SCENE_T_16_SHIFT = right_shift<int16_t>(MAX_SCENE_POSITION);
static constexpr const std::intmax_t SCENE_R_8_SHIFT = right_shift<int8_t>(1.1f * float(M_PI));
static constexpr const std::intmax_t SCENE_W_8_SHIFT = right_shift<int8_t>(2.f * float(M_PI) * 0.25f * rps);
static constexpr const std::intmax_t SCENE_V_8_SHIFT = right_shift<int8_t>(200.f * kph);
using CompressedSceneT16 = FixedPointNumber<int16_t, SCENE_T_16_SHIFT>;
using CompressedSceneR8 = FixedPointNumber<int8_t, SCENE_R_8_SHIFT>;
using CompressedSceneW8 = FixedPointNumber<int8_t, SCENE_W_8_SHIFT>;
using CompressedSceneV8 = FixedPointNumber<int8_t, SCENE_V_8_SHIFT>;

// Relative + highres
static constexpr float PING = 100.f * milli * seconds;
static constexpr const std::intmax_t PHYSICS_T_16_SHIFT = right_shift<int16_t>(200.f * kph * PING);
static constexpr const std::intmax_t PHYSICS_R_8_SHIFT = right_shift<int8_t>(2.f * float(M_PI) * 0.125f * rps * PING);
static constexpr const std::intmax_t PHYSICS_W_8_SHIFT = right_shift<int8_t>(2.f * float(M_PI) * 0.25f * rps / (1.f * seconds) * PING);
static constexpr const std::intmax_t PHYSICS_V_8_SHIFT = right_shift<int8_t>(100.f * kph / (1.f * seconds) * PING);
static constexpr const std::intmax_t DELTA_T_16_SHIFT = DELTA_RIGHT_SHIFT<int16_t, CompressedSceneT16> - 3;
static constexpr const std::intmax_t DELTA_R_8_SHIFT = DELTA_RIGHT_SHIFT<int8_t, CompressedSceneR8> - 3;
static constexpr const std::intmax_t DELTA_W_8_SHIFT = DELTA_RIGHT_SHIFT<int8_t, CompressedSceneW8> - 3;
static constexpr const std::intmax_t DELTA_V_8_SHIFT = DELTA_RIGHT_SHIFT<int8_t, CompressedSceneV8> - 3;
static_assert(DELTA_T_16_SHIFT <= PHYSICS_T_16_SHIFT);
static_assert(DELTA_R_8_SHIFT <= PHYSICS_R_8_SHIFT);
static_assert(DELTA_W_8_SHIFT <= PHYSICS_W_8_SHIFT);
static_assert(DELTA_V_8_SHIFT <= PHYSICS_V_8_SHIFT);
using DeltaSceneT16 = FixedPointNumber<int16_t, DELTA_T_16_SHIFT>;
using DeltaSceneR8 = FixedPointNumber<int8_t, DELTA_R_8_SHIFT>;
using DeltaSceneW8 = FixedPointNumber<int8_t, DELTA_W_8_SHIFT>;
using DeltaSceneV8 = FixedPointNumber<int8_t, DELTA_V_8_SHIFT>;

// Absolute + highres
using CompressedSceneT32 = DeltaSceneT16::ReplacedInt<int32_t>;
using CompressedSceneR16 = DeltaSceneR8::ReplacedInt<int16_t>;
using CompressedSceneW16 = DeltaSceneW8::ReplacedInt<int16_t>;
using CompressedSceneV16 = DeltaSceneV8::ReplacedInt<int16_t>;

}
