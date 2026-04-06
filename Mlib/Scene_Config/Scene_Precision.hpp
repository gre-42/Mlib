#pragma once
#include <Mlib/Hashing/Hash.hpp>
#include <Mlib/Hashing/Std_Hash.hpp>
#include <Mlib/Math/Fixed_Point_Number.hpp>

namespace Mlib {

using SceneDir = float;
using ScenePos = double;

static const std::intmax_t SCENE_POS_DENOMINATOR = (1 << 11);
using CompressedScenePos = FixedPointNumber<int32_t, SCENE_POS_DENOMINATOR>;
using HalfCompressedScenePos = FixedPointNumber<int16_t, SCENE_POS_DENOMINATOR>;

}

template <>
struct std::hash<Mlib::CompressedScenePos>
{
    std::size_t operator() (const Mlib::CompressedScenePos& a) const {
        return Mlib::hash_combine(a.count);
    }
};
