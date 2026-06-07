#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Writer.hpp>
#include <Mlib/Scene_Config/Physics_Precision.hpp>
#include <string_view>

namespace Mlib {

inline void serialize_ws_8(BinaryBitwiseWordsWriter& writer, const FixedArray<SceneDir, 3>& w, std::string_view message) {
    writer.serialize(w.casted<CompressedSceneW8>(), message);
}

inline void serialize_vs_8(BinaryBitwiseWordsWriter& writer, const FixedArray<SceneDir, 3>& v, std::string_view message) {
    writer.serialize(v.casted<CompressedSceneV8>(), message);
}

inline void serialize_w_8(BinaryBitwiseWordsWriter& writer, const SceneDir& w, std::string_view message) {
    writer.serialize((CompressedSceneW8)w, message);
}

}
