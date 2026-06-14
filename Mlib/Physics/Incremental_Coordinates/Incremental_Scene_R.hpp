#pragma once
#include <Mlib/Physics/Incremental_Coordinates/Incremental_Config.hpp>
#include <Mlib/Scene_Config/Physics_Precision.hpp>
#include <optional>
#include <sstream>

namespace Mlib {

inline CompressedSceneR16 upsample_angle(CompressedSceneR8 a8) {
    return (CompressedSceneR16)(a8);
}

inline CompressedSceneR16 plus_angle(CompressedSceneR8 base, DeltaSceneR8 delta) {
    // return CompressedSceneR16::from_float_safe((float)base + (float)delta);
    return (CompressedSceneR16)base + (CompressedSceneR16)delta;
}

inline DeltaSceneR8 minus_angle_unsafe(CompressedSceneR16 current, CompressedSceneR8 base) {
    constexpr const auto r16_pi = CompressedSceneR16{M_PI};
    constexpr const auto r16_2pi = CompressedSceneR16{2. * M_PI};
    auto diff = current - (CompressedSceneR16)base;
    if (diff > r16_pi) {
        diff -= r16_2pi;
    }
    if (diff < -r16_pi) {
        diff += r16_2pi;
    }
    // return DeltaSceneR8::from_float_safe((float)current - (float)old);
    return (DeltaSceneR8)diff;
}

inline DeltaSceneR8 minus_angle(CompressedSceneR16 current, CompressedSceneR8 base, IncrementalConfig& c) {
    auto delta = minus_angle_unsafe(current, base);
    auto recon = plus_angle(base, delta);
    float diff = std::abs((float)(recon - current));
    if (diff > M_PI) {
        diff -= float(2. * M_PI);
    }
    if (diff < -M_PI) {
        diff += float(2. * M_PI);
    }
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
