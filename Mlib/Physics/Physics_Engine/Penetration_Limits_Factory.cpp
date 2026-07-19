#include "Penetration_Limits_Factory.hpp"
#include <Mlib/Physics/Physics_Engine/Penetration_Limits.hpp>
#include <Mlib/Scene_Config/Physics_Precision.hpp>
#include <cmath>

using namespace Mlib;

PenetrationLimitsFactory::PenetrationLimitsFactory(
    float max_penetration,
    float radius)
    : max_penetration_{ max_penetration }
    , radius_{ radius }
{}

PenetrationLimitsFactory PenetrationLimitsFactory::inf() {
    return { INFINITY, 1.f };
}

float PenetrationLimitsFactory::vmax_translation(float dt) const {
    if (max_penetration_ == INFINITY) {
        return INFINITY;
    }
    return std::min(
        PenetrationLimits{dt, max_penetration_}.vmax_translation,
        MAX_REMOTE_VELOCITY);
}

float PenetrationLimitsFactory::wmax(float dt) const {
    if (max_penetration_ == INFINITY) {
        return INFINITY;
    }
    return std::min(
        PenetrationLimits{dt, max_penetration_}.wmax(radius_),
        MAX_REMOTE_ANGULAR_VELOCITY);
}
