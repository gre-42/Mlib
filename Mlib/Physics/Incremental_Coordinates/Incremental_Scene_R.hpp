#pragma once
#include <Mlib/Math/Angle_Pi_Pi.hpp>
#include <Mlib/Physics/Incremental_Coordinates/Incremental_Config.hpp>
#include <Mlib/Scene_Config/Physics_Precision.hpp>
#include <optional>
#include <sstream>

namespace Mlib {

inline CompressedSceneR16 fixed_point_angle(SceneDir a) {
    return (CompressedSceneR16)angle_pi_pi(a);
}

inline CompressedSceneR16 plus_angle(CompressedSceneR8 base, DeltaSceneR8 delta) {
    // return CompressedSceneR16::from_float_safe((float)base + (float)delta);
    return angle_pi_pi((CompressedSceneR16)base + (CompressedSceneR16)delta);
}

inline DeltaSceneR8 minus_angle_unsafe(CompressedSceneR16 current, CompressedSceneR8 base) {
    auto diff = angle_pi_pi(current - (CompressedSceneR16)base);
    // return DeltaSceneR8::from_float_safe((float)current - (float)old);
    return (DeltaSceneR8)diff;
}

inline DeltaSceneR8 minus_angle(CompressedSceneR16 current, CompressedSceneR8 base, IncrementalConfig& c) {
    auto delta = minus_angle_unsafe(current, base);
    auto recon = plus_angle(base, delta);
    float diff = std::abs((float)(angle_pi_pi(recon - current)));
    if (diff > 4.f * (float)std::numeric_limits<CompressedSceneR16>::min()) {
        if (any(c & IncrementalConfig::RAISE)) {
            throw std::runtime_error((std::stringstream() <<
                "Could not compress angle " << current <<
                " with base " << base <<
                ". Delta: " << delta <<
                ", recon: " << recon <<
                ", diff: " << (diff / degrees) << "degrees").str());
        }
        c |= IncrementalConfig::OVERFLOW;
    }
    return delta;
}

}
