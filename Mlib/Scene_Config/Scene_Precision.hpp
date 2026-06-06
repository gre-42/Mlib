#pragma once
#include <Mlib/Hashing/Hash.hpp>
#include <Mlib/Hashing/Std_Hash.hpp>
#include <Mlib/Math/Fixed_Point_Number.hpp>

namespace Mlib {

using SceneDir = float;
using ScenePos = double;

// Positions
// (2**31 - 1) / (1<<11) / 1e3 = 1048.576 => 1000 kilometers of range
// 1 / (1<<11) / 1e-3 = 0.48828125 => 0.5 millimeters resolution
static const std::intmax_t SCENE_POS_DENOMINATOR = (1 << 11);
using CompressedScenePos = FixedPointNumber<int32_t, SCENE_POS_DENOMINATOR>;
using HalfCompressedScenePos = FixedPointNumber<int16_t, SCENE_POS_DENOMINATOR>;

// Directions
// (2**15-1) / (1<<14) = 1.99 => within [0, 1]
// 1 / (1<<14) / 1e-3 = 0.06103515625 => 0.06 millimeters resolution
static const std::intmax_t SCENE_DIR_DENOMINATOR = (1 << 14);
using CompressedSceneDir = FixedPointNumber<int16_t, SCENE_DIR_DENOMINATOR>;

// Angles
// (2**15-1) / (1<<12) = 7.99 => within [0, 2*PI]
// 1 / (1<<12) * 180 / PI = 0.014 => 0.014 degrees resolution
static const std::intmax_t SCENE_ANGLE_DENOMINATOR = (1 << 13);
using CompressedSceneAngle = FixedPointNumber<int16_t, SCENE_ANGLE_DENOMINATOR>;

}

template <>
struct std::hash<Mlib::CompressedScenePos>
{
    std::size_t operator() (const Mlib::CompressedScenePos& a) const {
        return Mlib::hash_combine(a.count);
    }
};

template <>
struct std::hash<Mlib::CompressedSceneDir>
{
    std::size_t operator() (const Mlib::CompressedSceneDir& a) const {
        return Mlib::hash_combine(a.count);
    }
};

template <>
struct std::hash<Mlib::CompressedSceneAngle>
{
    std::size_t operator() (const Mlib::CompressedSceneAngle& a) const {
        return Mlib::hash_combine(a.count);
    }
};
