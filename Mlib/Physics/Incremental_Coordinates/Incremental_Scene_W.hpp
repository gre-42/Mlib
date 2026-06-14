#pragma once
#include <Mlib/Physics/Incremental_Coordinates/Incremental_Config.hpp>
#include <Mlib/Scene_Config/Physics_Precision.hpp>
#include <optional>
#include <sstream>

namespace Mlib {

inline CompressedSceneW16 upsample_angular_velocity(CompressedSceneW8 v8) {
    return (CompressedSceneW16)(v8);
}

inline CompressedSceneW16 plus_angular_velocity(CompressedSceneW8 base, DeltaSceneW8 delta) {
    // return CompressedSceneW16::from_float_safe((float)base + (float)delta);
    return (CompressedSceneW16)base + (CompressedSceneW16)delta;
}

inline DeltaSceneW8 minus_angular_velocity_unsafe(CompressedSceneW16 current, CompressedSceneW8 base) {
    // return DeltaSceneW8::from_float_safe((float)current - (float)base);
    return (DeltaSceneW8)(current - (CompressedSceneW16)base);
}

inline DeltaSceneW8 minus_angular_velocity(CompressedSceneW16 current, CompressedSceneW8 base, IncrementalConfig& c) {
    auto delta = minus_angular_velocity_unsafe(current, base);
    auto recon = plus_angular_velocity(base, delta);
    float diff = std::abs((float)(recon - current));
    if (diff > 1e-1f * rpm) {
        if (any(c & IncrementalConfig::RAISE)) {
            throw std::runtime_error((std::stringstream() <<
                "Could not compress angular velocity " << current <<
                " with base " << base <<
                ". Delta: " << delta <<
                ", recon: " << recon <<
                ", diff: " << (diff / rpm) << "rpm").str());
        }
        c |= IncrementalConfig::OVERFLOW;
    }
    return delta;
}

}
