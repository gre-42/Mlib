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
static constexpr const std::intmax_t SCENE_W_8_SHIFT = right_shift<int8_t>(1.f * rps);
static constexpr const std::intmax_t SCENE_V_8_SHIFT = right_shift<int8_t>(200.f * kph);
using CompressedSceneT16 = FixedPointNumber<int16_t, SCENE_T_16_SHIFT>;
using CompressedSceneR8 = FixedPointNumber<int8_t, SCENE_R_8_SHIFT>;
using CompressedSceneW8 = FixedPointNumber<int8_t, SCENE_W_8_SHIFT>;
using CompressedSceneV8 = FixedPointNumber<int8_t, SCENE_V_8_SHIFT>;

// Absolute + highres
static constexpr const std::intmax_t SCENE_T_32_SHIFT = SCENE_POS_SHIFT;
static constexpr const std::intmax_t SCENE_R_16_SHIFT = right_shift<int16_t>(2.1f * float(M_PI));
static constexpr const std::intmax_t SCENE_W_16_SHIFT = right_shift<int16_t>(1.f * rps);
static constexpr const std::intmax_t SCENE_V_16_SHIFT = right_shift<int16_t>(200.f * kph);
using CompressedSceneT32 = CompressedScenePos;
using CompressedSceneR16 = FixedPointNumber<int16_t, SCENE_R_16_SHIFT>;
using CompressedSceneW16 = FixedPointNumber<int16_t, SCENE_W_16_SHIFT>;
using CompressedSceneV16 = FixedPointNumber<int16_t, SCENE_V_16_SHIFT>;

// Relative + highres
using DeltaSceneT16 = DeltaFixedPointNumberT<int16_t, CompressedSceneT16>;
using DeltaSceneR8 = DeltaFixedPointNumberT<int8_t, CompressedSceneR8>;
using DeltaSceneW8 = DeltaFixedPointNumberT<int8_t, CompressedSceneW8>;
using DeltaSceneV8 = DeltaFixedPointNumberT<int8_t, CompressedSceneV8>;

}
