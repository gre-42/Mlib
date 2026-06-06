#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Writer.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <string_view>

namespace Mlib {

inline void serialize_position(BinaryBitwiseWordsWriter& writer, const FixedArray<ScenePos, 3>& pos, std::string_view message) {
    writer.serialize(pos.casted<CompressedScenePos>(), message);
}

inline void serialize_direction(BinaryBitwiseWordsWriter& writer, const FixedArray<SceneDir, 3>& dir, std::string_view message) {
    SceneDir len = std::sqrt(sum(squared(dir)));
    if (len < 1e-12) {
        writer.write_binary(0.f, message);
        writer.serialize(fixed_zeros<CompressedSceneDir, 3>(), message);
    } else {
        writer.write_binary(len, message);
        writer.serialize((dir / len).casted<CompressedSceneDir>(), message);
    }
}

inline void serialize_angles(BinaryBitwiseWordsWriter& writer, const FixedArray<SceneDir, 3>& angles, std::string_view message) {
    writer.serialize(angles.casted<CompressedSceneAngle>(), message);
}

}
