#include "Penetration_Limits_Factory.hpp"
#include <Mlib/Physics/Physics_Engine/Penetration_Limits.hpp>
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
    return PenetrationLimits{dt, max_penetration_}.vmax_translation;
}
float PenetrationLimitsFactory::wmax(float dt) const {
    return PenetrationLimits{dt, max_penetration_}.wmax(radius_);
}
