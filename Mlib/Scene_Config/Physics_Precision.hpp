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
static constexpr const std::intmax_t SCENE_R_8_SHIFT = right_shift<int8_t>(2.1f * float(M_PI));
static constexpr const std::intmax_t SCENE_W_8_SHIFT = right_shift<int8_t>(2.f * float(M_PI) * 0.25f * rps);
static constexpr const std::intmax_t SCENE_V_8_SHIFT = right_shift<int8_t>(200.f * kph);
using CompressedSceneT16 = FixedPointNumber<int16_t, SCENE_T_16_SHIFT>;
using CompressedSceneR8 = FixedPointNumber<int8_t, SCENE_R_8_SHIFT>;
using CompressedSceneW8 = FixedPointNumber<int8_t, SCENE_W_8_SHIFT>;
using CompressedSceneV8 = FixedPointNumber<int8_t, SCENE_V_8_SHIFT>;

// Absolute + highres
static constexpr const std::intmax_t SCENE_T_32_SHIFT = SCENE_POS_SHIFT;
static constexpr const std::intmax_t SCENE_R_16_SHIFT = right_shift<int16_t>(2.1f * float(M_PI));
static constexpr const std::intmax_t SCENE_W_16_SHIFT = right_shift<int16_t>(0.25f * rps);
static constexpr const std::intmax_t SCENE_V_16_SHIFT = right_shift<int16_t>(200.f * kph);
using CompressedSceneT32 = CompressedScenePos;
using CompressedSceneR16 = FixedPointNumber<int16_t, SCENE_R_16_SHIFT>;
using CompressedSceneW16 = FixedPointNumber<int16_t, SCENE_W_16_SHIFT>;
using CompressedSceneV16 = FixedPointNumber<int16_t, SCENE_V_16_SHIFT>;

// Relative + highres
static constexpr float PING = 100.f * milli * seconds;
static constexpr const std::intmax_t PHYSICS_T_16_SHIFT = right_shift<int16_t>(200.f * kph * PING);
static constexpr const std::intmax_t PHYSICS_R_8_SHIFT = right_shift<int8_t>(2.f * float(M_PI) * 0.25f * rps * PING);
static constexpr const std::intmax_t PHYSICS_W_8_SHIFT = right_shift<int8_t>(2.f * float(M_PI) * 0.25f * rps / (1.f * seconds) * PING);
static constexpr const std::intmax_t PHYSICS_V_8_SHIFT = right_shift<int8_t>(100.f * kph / (1.f * seconds) * PING);
static constexpr const std::intmax_t DELTA_T_16_SHIFT = DELTA_RIGHT_SHIFT<int16_t, CompressedSceneT16>;
static constexpr const std::intmax_t DELTA_R_8_SHIFT = DELTA_RIGHT_SHIFT<int8_t, CompressedSceneR8>;
static constexpr const std::intmax_t DELTA_W_8_SHIFT = DELTA_RIGHT_SHIFT<int8_t, CompressedSceneW8>;
static constexpr const std::intmax_t DELTA_V_8_SHIFT = DELTA_RIGHT_SHIFT<int8_t, CompressedSceneV8>;
using DeltaSceneT16 = FixedPointNumber<int16_t, std::min(DELTA_T_16_SHIFT, PHYSICS_T_16_SHIFT)>;
using DeltaSceneR8 = FixedPointNumber<int8_t, std::min(DELTA_R_8_SHIFT, PHYSICS_R_8_SHIFT)>;
using DeltaSceneW8 = FixedPointNumber<int8_t, std::min(DELTA_W_8_SHIFT, PHYSICS_W_8_SHIFT)>;
using DeltaSceneV8 = FixedPointNumber<int8_t, std::min(DELTA_V_8_SHIFT, PHYSICS_V_8_SHIFT)>;

}
