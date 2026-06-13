#pragma once
#include <Mlib/Scene_Config/Physics_Precision.hpp>

namespace Mlib {

inline CompressedSceneR16 upsample_angle(CompressedSceneR8 a8) {
    return (CompressedSceneR16)(a8);
}

inline CompressedSceneR16 plus_angle(CompressedSceneR8 base, DeltaSceneR8 delta) {
    // return CompressedSceneR16::from_float_safe((float)base + (float)delta);
    return (CompressedSceneR16)base + (CompressedSceneR16)(delta);
}

inline DeltaSceneR8 minus_angle(CompressedSceneR16 current, CompressedSceneR8 base) {
    // return DeltaSceneR8::from_float_safe((float)current - (float)old);
    return (DeltaSceneR8)(current - (CompressedSceneR16)base);
}

}
