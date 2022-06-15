#include "Rigid_Body_Engine.hpp"
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Signed_Min.hpp>
#include <Mlib/Physics/Actuators/Engine_Event_Listener.hpp>
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
  ntires_old_{ 0 },
  hand_brake_pulled_{ hand_brake_pulled },
  audio_{ audio }
{}

RigidBodyEngine::~RigidBodyEngine()
{}

void RigidBodyEngine::reset_forces() {
    engine_state = EngineState::OFF;
    ntires_old_ = tires_consumed_.size();
    tires_consumed_.clear();
}

PowerIntent RigidBodyEngine::consume_abs_surface_power(size_t tire_id, float w) {
    if (!tires_consumed_.insert(tire_id).second ||
        (tires_consumed_.size() > ntires_old_)) {
        return PowerIntent{.power = 0, .type = PowerIntentType::ALWAYS_IDLE};
    }

    float max_surface_power = (std::isnan(w_) || (ntires_old_ == 0))
        ? 0.f
        : engine_power_.get_power(w);
    if (hand_brake_pulled_ || std::isnan(surface_power_)) {
        return PowerIntent{.power = NAN, .type = PowerIntentType::ALWAYS_BREAK};
    } else if (max_surface_power == 0) {
        if (delta_power_ == 0) {
            return PowerIntent{.power = sign(surface_power_), .type = PowerIntentType::BREAK_OR_IDLE};
        } else {
            return PowerIntent{.power = sign(delta_power_), .type = PowerIntentType::BREAK_OR_IDLE};
        }
    } else {
        auto clip_power = [&max_surface_power](float p){
            return signed_min(p, max_surface_power);
        };
        float sp = clip_power(clip_power(surface_power_) + delta_power_);
        return PowerIntent{.power = sp / float(ntires_old_), .type = PowerIntentType::ACCELERATE_OR_BREAK};
    }
}

float RigidBodyEngine::surface_power() const {
    return surface_power_;
}

void RigidBodyEngine::set_surface_power(float surface_power, float delta_power) {
    surface_power_ = surface_power;
    delta_power_ = delta_power;
}

void RigidBodyEngine::advance_time(float dt, const FixedArray<double, 3>& position) {
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
        audio_->set_position(position.casted<float>());
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

std::ostream& Mlib::operator << (std::ostream& ostr, const PowerIntent& power_intent) {
    ostr << "Power intent\n";
    ostr << "   intent type " << (int)power_intent.type << std::endl;
    ostr << "   power " << power_intent.power << std::endl;
    return ostr;
}

std::ostream& Mlib::operator << (std::ostream& ostr, const RigidBodyEngine& engine) {
    ostr << "Engine\n";
    ostr << "   engine_state " << (int)engine.engine_state << '\n';
    ostr << "   w " << engine.w_ << '\n';
    ostr << "   surface_power " << engine.surface_power_ << '\n';
    ostr << "   delta_power " << engine.delta_power_ << '\n';
    for (size_t c : engine.tires_consumed_) {
        ostr << "   consumed " << c << '\n';
    }
    ostr << engine.engine_power_ << '\n';
    ostr << "   ntires_old " << engine.ntires_old_ << '\n';
    ostr << "   hand_brake_pulled " << (int)engine.hand_brake_pulled_ << '\n';
    return ostr;
}
