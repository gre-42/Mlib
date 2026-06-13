#pragma once
#include <Mlib/Hashing/Hash.hpp>
#include <Mlib/Hashing/Std_Hash.hpp>
#include <Mlib/Math/Fixed_Point_Number.hpp>
#include <Mlib/Math/Int_Log2.hpp>
#include <Mlib/Physics/Units.hpp>

namespace Mlib {

template <std::integral T>
constexpr int64_t right_shift(double max) {
    return int_log2(std::numeric_limits<T>::max()) - int_log2(static_cast<int64_t>(max + 0.5));
}

template <std::integral T, class TLarge>
struct DeltaFixedPointNumber {
    using value_type = FixedPointNumber<T, right_shift<T>((double)std::numeric_limits<TLarge>::min())>;
};

template <std::integral T, class TLarge>
using DeltaFixedPointNumberT = DeltaFixedPointNumber<T, TLarge>::value_type;

using SceneDir = float;
using ScenePos = double;

static constexpr const float MAX_SCENE_POSITION = 1000.f * kilo * meters;

// Positions
// (2**31 - 1) / (1<<11) / 1e3 = 1048.576 => 1000 kilometers of range
// 1 / (1<<11) / 1e-3 = 0.48828125 => 0.5 millimeters resolution
static constexpr const std::intmax_t SCENE_POS_SHIFT = right_shift<int32_t>(MAX_SCENE_POSITION);
using CompressedScenePos = FixedPointNumber<int32_t, SCENE_POS_SHIFT>;
using HalfCompressedScenePos = FixedPointNumber<int16_t, SCENE_POS_SHIFT>;

}

template <>
struct std::hash<Mlib::CompressedScenePos>
{
    std::size_t operator() (const Mlib::CompressedScenePos& a) const {
        return Mlib::hash_combine(a.count);
    }
};
