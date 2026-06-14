#pragma once
#include <Mlib/Physics/Incremental_Coordinates/Incremental_Config.hpp>
#include <Mlib/Scene_Config/Physics_Precision.hpp>
#include <optional>
#include <sstream>

namespace Mlib {

inline CompressedSceneV16 upsample_velocity(CompressedSceneV8 v8) {
    return (CompressedSceneV16)(v8);
}

inline CompressedSceneV16 plus_velocity(CompressedSceneV8 base, DeltaSceneV8 delta) {
    // return CompressedSceneV16::from_float_safe((float)base + (float)delta);
    return (CompressedSceneV16)base + (CompressedSceneV16)delta;
}

inline DeltaSceneV8 minus_velocity_unsafe(CompressedSceneV16 current, CompressedSceneV8 base) {
    // return DeltaSceneV8::from_float_safe((float)current - (float)base);
    return (DeltaSceneV8)(current - (CompressedSceneV16)base);
}

inline DeltaSceneV8 minus_velocity(CompressedSceneV16 current, CompressedSceneV8 base, IncrementalConfig& c) {
    auto delta = minus_velocity_unsafe(current, base);
    auto recon = plus_velocity(base, delta);
    float diff = std::abs((float)(recon - current));
    if (diff > 2.f * kph) {
        if (any(c & IncrementalConfig::RAISE)) {
            throw std::runtime_error((std::stringstream() <<
                "Could not compress velocity " << current <<
                " with base " << base <<
                ". Delta: " << delta <<
                ", recon: " << recon <<
                ", diff: " << (diff / kph) << "kph").str());
        }
        c |= IncrementalConfig::OVERFLOW;
    }
    return delta;
}

}
