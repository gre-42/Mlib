#pragma once
#include <Mlib/Scene_Config/Physics_Precision.hpp>

namespace Mlib {

inline CompressedSceneW16 upsample_angular_velocity(CompressedSceneW8 v8) {
    return (CompressedSceneW16)(v8);
}

inline CompressedSceneW16 plus_angular_velocity(CompressedSceneW8 base, DeltaSceneW8 delta) {
    // return CompressedSceneW16::from_float_safe((float)base + (float)delta);
    return (CompressedSceneW16)base + (CompressedSceneW16)delta;
}

inline DeltaSceneW8 minus_angular_velocity(CompressedSceneW16 current, CompressedSceneW8 base) {
    // return DeltaSceneW8::from_float_safe((float)current - (float)base);
    return (DeltaSceneW8)(current - (CompressedSceneW16)base);
}

}
