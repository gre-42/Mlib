#include "Penetration_Limits_Factory.hpp"
#include <Mlib/Physics/Physics_Engine/Limit_Sources.hpp>
#include <Mlib/Physics/Physics_Engine/Penetration_Limits.hpp>
#include <Mlib/Scene_Config/Physics_Precision.hpp>
#include <cmath>

using namespace Mlib;

PenetrationLimitsFactory::PenetrationLimitsFactory(
    float max_penetration,
    float radius,
    LimitSources limit_sources)
    : max_penetration_{ max_penetration }
    , radius_{ radius }
    , limit_sources_{ limit_sources }
{}

PenetrationLimitsFactory PenetrationLimitsFactory::inf() {
    return { INFINITY, 1.f, LimitSources::NONE };
}

float PenetrationLimitsFactory::vmax_translation(float dt) const {
    float result = INFINITY;
    if (any(limit_sources_ & LimitSources::PENETRATION)) {
        result = std::min(result, PenetrationLimits{dt, max_penetration_}.vmax_translation);
    }
    if (any(limit_sources_ & LimitSources::REMOTE)) {
        result = std::min(result, MAX_REMOTE_VELOCITY);
    }
    return result;
}

float PenetrationLimitsFactory::wmax(float dt) const {
    float result = INFINITY;
    if (any(limit_sources_ & LimitSources::PENETRATION)) {
        result = std::min(result, PenetrationLimits{dt, max_penetration_}.wmax(radius_));
    }
    if (any(limit_sources_ & LimitSources::REMOTE)) {
        result = std::min(result, MAX_REMOTE_ANGULAR_VELOCITY);
    }
    return result;
}
