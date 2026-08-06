#pragma once
#include <Mlib/Math/Fixed_Point_Number.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <cstddef>
#include <limits>

namespace Mlib {

static constexpr const float MAX_REMOTE_VELOCITY = 200.f * kph;
static constexpr const float MAX_REMOTE_ANGULAR_VELOCITY = 0.25f * rps;

// Absolute + lowres
static constexpr const std::intmax_t SCENE_T_16_SHIFT = right_shift<int16_t>(MAX_SCENE_POSITION);
static constexpr const std::intmax_t SCENE_R_8_SHIFT = right_shift<int8_t>(1.1f * float(M_PI));
static constexpr const std::intmax_t SCENE_W_8_SHIFT = right_shift<int8_t>(MAX_REMOTE_ANGULAR_VELOCITY);
static constexpr const std::intmax_t SCENE_V_8_SHIFT = right_shift<int8_t>(MAX_REMOTE_VELOCITY);
using CompressedSceneT16 = FixedPointNumber<int16_t, SCENE_T_16_SHIFT>;
using CompressedSceneR8 = FixedPointNumber<int8_t, SCENE_R_8_SHIFT>;
using CompressedSceneW8 = FixedPointNumber<int8_t, SCENE_W_8_SHIFT>;
using CompressedSceneV8 = FixedPointNumber<int8_t, SCENE_V_8_SHIFT>;

// Relative + highres
static constexpr float PING = 250.f * milli * seconds;
struct DeltaSceneT16Width { static constexpr const double width = MAX_REMOTE_VELOCITY * PING; };
struct DeltaSceneR8Width { static constexpr const double width = 0.125f * rps * PING; };
struct DeltaSceneW8Width { static constexpr const double width = MAX_REMOTE_ANGULAR_VELOCITY / (1.f * seconds) * PING; };
struct DeltaSceneV8Width { static constexpr const double width = MAX_REMOTE_ANGULAR_VELOCITY / (1.f * seconds) * PING; };
using DeltaSceneT16 = DeltaFixedPointNumberT<int16_t, CompressedSceneT16, DeltaSceneT16Width>;
using DeltaSceneR8 = DeltaFixedPointNumberT<int8_t, CompressedSceneR8, DeltaSceneR8Width>;
using DeltaSceneW8 = DeltaFixedPointNumberT<int8_t, CompressedSceneW8, DeltaSceneW8Width>;
using DeltaSceneV8 = DeltaFixedPointNumberT<int8_t, CompressedSceneV8, DeltaSceneV8Width>;

// Absolute + highres
using CompressedSceneT32 = DeltaSceneT16::ReplacedInt<int32_t>;
using CompressedSceneR16 = DeltaSceneR8::ReplacedInt<int16_t>;
using CompressedSceneW16 = DeltaSceneW8::ReplacedInt<int16_t>;
using CompressedSceneV16 = DeltaSceneV8::ReplacedInt<int16_t>;

}
