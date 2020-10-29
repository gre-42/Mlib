#include "Rigid_Body_Engine.hpp"
#include <Mlib/Math/Fixed_Math.hpp>
#include <cmath>

using namespace Mlib;

RigidBodyEngine::RigidBodyEngine(float max_surface_power)
: surface_power_{0},
  surface_power_nconsumed_{0},
  max_surface_power_{max_surface_power},
  ntires_{0}
{}

void RigidBodyEngine::reset_forces() {
    surface_power_nconsumed_ = 0;
}

float RigidBodyEngine::consume_abs_surface_power() {
    if (std::isnan(surface_power_)) {
        return NAN;
    }
    if (surface_power_nconsumed_ >= ntires_) {
        return 0;
    }
    ++surface_power_nconsumed_;
    return surface_power_ / float(ntires_);
}

void RigidBodyEngine::set_surface_power(float surface_power) {
    if (std::isnan(surface_power)) {
        surface_power_ = NAN;
    } else {
        surface_power_ = sign(surface_power) * std::min(max_surface_power_, std::abs(surface_power));
    }
}

void RigidBodyEngine::increment_ntires() {
    ++ntires_;
}
