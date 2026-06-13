#pragma once
#include <Mlib/Scene_Config/Physics_Precision.hpp>

namespace Mlib {

inline CompressedSceneV16 upsample_velocity(CompressedSceneV8 v8) {
    return (CompressedSceneV16)(v8);
}

inline CompressedSceneV16 plus_velocity(CompressedSceneV8 base, DeltaSceneV8 delta) {
    // return CompressedSceneV16::from_float_safe((float)base + (float)delta);
    return (CompressedSceneV16)base + (CompressedSceneV16)(delta);
}

inline DeltaSceneV8 minus_velocity(CompressedSceneV16 current, CompressedSceneV8 base) {
    // return DeltaSceneV8::from_float_safe((float)current - (float)base);
    return (DeltaSceneV8)(current - (CompressedSceneV16)base);
}

}
