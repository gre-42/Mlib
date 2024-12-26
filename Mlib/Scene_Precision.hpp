#pragma once
#include <Mlib/Hash.hpp>
#include <Mlib/Math/Fixed_Point_Number.hpp>
#include <Mlib/Std_Hash.hpp>

namespace Mlib {

using SceneDir = float;
using ScenePos = double;

using CompressedScenePos = FixedPointNumber<int32_t, (1 << 11)>;
using HalfCompressedScenePos = FixedPointNumber<int16_t, (1 << 11)>;

}

template <>
struct std::hash<Mlib::CompressedScenePos>
{
    std::size_t operator() (const Mlib::CompressedScenePos& a) const {
        return Mlib::hash_combine(a.count);
    }
};
