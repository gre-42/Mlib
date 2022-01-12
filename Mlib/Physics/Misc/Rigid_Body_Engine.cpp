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
    const EnginePower& engine_power,
    bool hand_brake_pulled,
    const std::shared_ptr<EngineEventListener>& audio)
: engine_state{ EngineState::OFF },
  w_{ NAN },
  surface_power_{ 0 },
  delta_power_{ 0 },
  engine_power_{ engine_power },
  ntires_{ 0 },
  hand_brake_pulled_{ hand_brake_pulled },
  audio_{ audio }
{}

RigidBodyEngine::~RigidBodyEngine()
{}

void RigidBodyEngine::reset_forces() {
    engine_state = EngineState::OFF;
    tires_consumed_.clear();
}

PowerIntent RigidBodyEngine::consume_abs_surface_power(size_t tire_id, float w) {
    if (!tires_consumed_.insert(tire_id).second) {
        return PowerIntent{.power = 0, .type = PowerIntentType::ALWAYS_IDLE};
    }
    if (tires_consumed_.size() > ntires_) {
        throw std::runtime_error("Consumed surface power more often than number of tires");
    }

    float max_surface_power = std::isnan(w_)
        ? 0.f
        : engine_power_.get_power(w);
    if (hand_brake_pulled_ || std::isnan(surface_power_)) {
        return PowerIntent{.power = NAN, .type = PowerIntentType::ALWAYS_BREAK};
    } else if (max_surface_power == 0) {
        return PowerIntent{.power = sign(surface_power_ + delta_power_), .type = PowerIntentType::BREAK_OR_IDLE};
    } else {
        auto clip_power = [&max_surface_power](float p){
            return sign(p) * std::min(max_surface_power, std::abs(p));
        };
        float sp = clip_power(clip_power(surface_power_) + delta_power_);
        return PowerIntent{.power = sp / float(ntires_), .type = PowerIntentType::ACCELERATE_OR_BREAK};
    }
}

float RigidBodyEngine::surface_power() const {
    return surface_power_;
}

void RigidBodyEngine::set_surface_power(float surface_power, float delta_power) {
    surface_power_ = surface_power;
    delta_power_ = delta_power;
}

void RigidBodyEngine::increment_ntires() {
    ++ntires_;
}

void RigidBodyEngine::advance_time(float dt, const FixedArray<float, 3>& position) {
    if (!std::isnan(w_)) {
        engine_power_.auto_set_gear(w_);
    }
    if (audio_ != nullptr) {
        if ((engine_state == EngineState::OFF) || hand_brake_pulled_ || std::isnan(w_)) {
            audio_->notify_off();
        } else if (engine_state == EngineState::IDLE) {
            audio_->notify_idle(engine_power_.engine_w(w_));
        } else if (engine_state == EngineState::ACCELERATE) {
            audio_->notify_driving(engine_power_.engine_w(w_));
        }
        audio_->set_position(position);
    }
}

void RigidBodyEngine::notify_off() {
    engine_state = EngineState::OFF;
    w_ = 0.f;
}

void RigidBodyEngine::notify_idle(float w) {
    engine_state = EngineState::IDLE;
    w_ = w;
}

void RigidBodyEngine::notify_accelerate(float w) {
    engine_state = EngineState::ACCELERATE;
    w_ = w;
}
