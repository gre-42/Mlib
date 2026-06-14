#pragma once
#include <Mlib/Physics/Incremental_Coordinates/Incremental_Config.hpp>
#include <Mlib/Scene_Config/Physics_Precision.hpp>
#include <optional>
#include <sstream>

namespace Mlib {

inline CompressedSceneT32 upsample_position(CompressedSceneT16 t16) {
    return (CompressedSceneT32)(t16);
}

inline CompressedSceneT32 plus_position(CompressedSceneT16 base, DeltaSceneT16 delta) {
    // return CompressedSceneT32::from_float_safe((float)base + (float)delta);
    return (CompressedSceneT32)base + (CompressedSceneT32)delta;
}

inline DeltaSceneT16 minus_position_unsafe(CompressedSceneT32 current, CompressedSceneT16 base) {
    // return DeltaSceneT16::from_float_safe((float)current - (float)base);
    return (DeltaSceneT16)(current - (CompressedSceneT32)base);
}

inline DeltaSceneT16 minus_position(CompressedSceneT32 current, CompressedSceneT16 base, IncrementalConfig& c) {
    auto delta = minus_position_unsafe(current, base);
    auto recon = plus_position(base, delta);
    float diff = std::abs((float)(recon - current));
    if (diff > 1e-1f * meters) {
        if (any(c & IncrementalConfig::RAISE)) {
            throw std::runtime_error((std::stringstream() <<
                "Could not compress position " << current <<
                " with base " << base <<
                ". Delta: " << delta <<
                ", recon: " << recon <<
                ", diff: " << (diff / meters) << 'm').str());
        }
        c |= IncrementalConfig::OVERFLOW;
    }
    return delta;
}

}
