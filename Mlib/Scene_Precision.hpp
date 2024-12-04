#pragma once
#include <Mlib/Hash.hpp>
#include <Mlib/Scaled_Integer.hpp>
#include <Mlib/Std_Hash.hpp>

namespace Mlib {

using SceneDir = float;
using ScenePos = double;

using CompressedScenePos = ScaledInteger<int32_t, 1, (1 << 11)>;
using HalfCompressedScenePos = ScaledInteger<int16_t, 1, (1 << 11)>;

}

template <>
struct std::hash<Mlib::CompressedScenePos>
{
    std::size_t operator() (const Mlib::CompressedScenePos& a) const {
        return Mlib::hash_combine(a.count);
    }
};
