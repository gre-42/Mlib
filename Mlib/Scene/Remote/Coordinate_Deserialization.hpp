#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Reader.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <string_view>

namespace Mlib {

inline FixedArray<ScenePos, 3> deserialize_position(BinaryBitwiseWordsReader& reader, std::string_view message) {
    return reader.deserialize<EFixedArray<CompressedScenePos, 3>>(message).casted<ScenePos>();
}

inline FixedArray<SceneDir, 3> deserialize_direction(BinaryBitwiseWordsReader& reader, std::string_view message) {
    auto len = reader.read_binary<SceneDir>(message);
    return reader.deserialize<EFixedArray<CompressedSceneDir, 3>>(message).casted<SceneDir>() * len;
}

inline FixedArray<SceneDir, 3> deserialize_angles(BinaryBitwiseWordsReader& reader, std::string_view message) {
    return reader.deserialize<EFixedArray<CompressedSceneAngle, 3>>(message).casted<SceneDir>();
}

}
