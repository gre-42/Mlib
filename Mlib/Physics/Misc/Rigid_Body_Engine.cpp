#include "Rigid_Body_Engine.hpp"
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Physics/Misc/Engine_Event_Listener.hpp>
#include <cmath>

using namespace Mlib;

RigidBodyEngine::RigidBodyEngine(
    float max_surface_power,
    bool hand_brake_pulled,
    const std::shared_ptr<EngineEventListener>& audio)
: surface_power_{0},
  surface_power_nconsumed_{0},
  surface_power_consumed_{0},
  max_surface_power_{max_surface_power},
  ntires_{0},
  hand_brake_pulled_{hand_brake_pulled},
  audio_{audio}
{}

RigidBodyEngine::~RigidBodyEngine()
{}

void RigidBodyEngine::reset_forces() {
    surface_power_consumed_ = 0.f;
    surface_power_nconsumed_ = 0;
}

PowerIntent RigidBodyEngine::consume_abs_surface_power() {
    if (hand_brake_pulled_ || std::isnan(surface_power_)) {
        return PowerIntent{.power = NAN, .type = PowerIntentType::ALWAYS_BREAK};
    }
    if (max_surface_power_ == 0) {
        return PowerIntent{.power = surface_power_, .type = PowerIntentType::BREAK_OR_IDLE};
    }
    if (surface_power_nconsumed_ >= ntires_) {
        throw std::runtime_error("Consumed surface power more often than number of tires");
    }
    ++surface_power_nconsumed_;
    float power = surface_power_ / float(ntires_);
    surface_power_consumed_ += power;
    return PowerIntent{.power = power, .type = PowerIntentType::ACCELERATE_OR_BREAK};
}

void RigidBodyEngine::set_surface_power(float surface_power) {
    if (std::isnan(surface_power)) {
        surface_power_ = NAN;
    } else if (max_surface_power_ == 0) {
        surface_power_ = sign(surface_power);
    } else {
        surface_power_ = sign(surface_power) * std::min(max_surface_power_, std::abs(surface_power));
    }
}

void RigidBodyEngine::increment_ntires() {
    ++ntires_;
}

void RigidBodyEngine::advance_time(float dt) {
    if (audio_ != nullptr) {
        if (surface_power_consumed_ != 0) {
            audio_->notify_driving();
        } else {
            audio_->notify_idle();
        }
    }
}
