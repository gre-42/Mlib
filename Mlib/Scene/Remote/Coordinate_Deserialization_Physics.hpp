#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Reader.hpp>
#include <Mlib/Scene_Config/Physics_Precision.hpp>
#include <string_view>

namespace Mlib {

inline FixedArray<SceneDir, 3> deserialize_ws_8(BinaryBitwiseWordsReader& reader, std::string_view message) {
    return reader.deserialize<EFixedArray<CompressedSceneW8, 3>>(message).casted<SceneDir>();
}

inline FixedArray<SceneDir, 3> deserialize_vs_8(BinaryBitwiseWordsReader& reader, std::string_view message) {
    return reader.deserialize<EFixedArray<CompressedSceneV8, 3>>(message).casted<SceneDir>();
}

inline SceneDir deserialize_w_8(BinaryBitwiseWordsReader& reader, std::string_view message) {
    return (SceneDir)reader.read_binary<CompressedSceneW8>(message);
}

}
