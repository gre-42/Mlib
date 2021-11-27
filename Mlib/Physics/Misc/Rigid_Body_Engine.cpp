#include "Rigid_Body_Engine.hpp"
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Physics/Misc/Engine_Event_Listener.hpp>
#include <cmath>

using namespace Mlib;

namespace Mlib {

enum class EngineState {
    OFF,
    IDLE,
    ACCELERATE
};

}

RigidBodyEngine::RigidBodyEngine(
    float max_surface_power,
    bool hand_brake_pulled,
    const std::shared_ptr<EngineEventListener>& audio)
: engine_state{EngineState::OFF},
  surface_power_{0},
  max_surface_power_{max_surface_power},
  ntires_{0},
  hand_brake_pulled_{hand_brake_pulled},
  audio_{audio}
{}

RigidBodyEngine::~RigidBodyEngine()
{}

void RigidBodyEngine::reset_forces() {
    engine_state = EngineState::OFF;
    tires_consumed_.clear();
}

PowerIntent RigidBodyEngine::consume_abs_surface_power(size_t tire_id) {
    if (!tires_consumed_.insert(tire_id).second) {
        return PowerIntent{.power = 0, .type = PowerIntentType::ALWAYS_IDLE};
    }
    if (tires_consumed_.size() > ntires_) {
        throw std::runtime_error("Consumed surface power more often than number of tires");
    }
    if (hand_brake_pulled_ || std::isnan(surface_power_)) {
        return PowerIntent{.power = NAN, .type = PowerIntentType::ALWAYS_BREAK};
    }
    if (max_surface_power_ == 0) {
        return PowerIntent{.power = surface_power_, .type = PowerIntentType::BREAK_OR_IDLE};
    }
    return PowerIntent{.power = surface_power_ / float(ntires_), .type = PowerIntentType::ACCELERATE_OR_BREAK};
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
        if ((engine_state == EngineState::OFF) || hand_brake_pulled_) {
            audio_->notify_off();
        } else if (engine_state == EngineState::IDLE) {
            audio_->notify_idle(w_);
        } else if (engine_state == EngineState::ACCELERATE) {
            audio_->notify_driving(w_);
        }
    }
}

void RigidBodyEngine::notify_off() {
    engine_state = EngineState::OFF;
}

void RigidBodyEngine::notify_idle(float w) {
    engine_state = EngineState::IDLE;
    w_ = w;
}

void RigidBodyEngine::notify_accelerate(float w) {
    engine_state = EngineState::ACCELERATE;
    w_ = w;
}
