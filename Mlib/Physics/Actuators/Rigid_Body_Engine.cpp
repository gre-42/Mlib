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
: engine_state_{ EngineState::OFF },
  w_{ NAN },
  engine_power_intent_{
    .surface_power = 0.f,
    .delta_power = 0.f,
    .relaxation = 0.f},
  engine_power_{ engine_power },
  ntires_old_{ 0 },
  hand_brake_pulled_{ hand_brake_pulled },
  audio_{ audio }
{}

RigidBodyEngine::~RigidBodyEngine()
{}

void RigidBodyEngine::reset_forces() {
    engine_state_ = EngineState::OFF;
    ntires_old_ = tires_consumed_.size();
    tires_consumed_.clear();
}

TirePowerIntent RigidBodyEngine::consume_abs_surface_power(size_t tire_id, float w) {
    if (!tires_consumed_.insert(tire_id).second ||
        (tires_consumed_.size() > ntires_old_)) {
        return TirePowerIntent{
            .power = 0,
            .relaxation = 1.f,
            .type = TirePowerIntentType::ALWAYS_IDLE};
    }

    float max_surface_power = (std::isnan(w_) || (ntires_old_ == 0))
        ? 0.f
        : engine_power_.get_power(w);
    if (hand_brake_pulled_ || std::isnan(engine_power_intent_.surface_power) || std::isnan(max_surface_power)) {
        return TirePowerIntent{
            .power = NAN,
            .relaxation = engine_power_intent_.relaxation,
            .type = TirePowerIntentType::ALWAYS_BREAK};
    } else if (max_surface_power == 0) {
        if (engine_power_intent_.delta_power == 0) {
            return TirePowerIntent{
                .power = sign(engine_power_intent_.surface_power),
                .relaxation = engine_power_intent_.relaxation,
                .type = TirePowerIntentType::BRAKE_OR_IDLE};
        } else {
            return TirePowerIntent{
                .power = sign(engine_power_intent_.delta_power),
                .relaxation = engine_power_intent_.relaxation,
                .type = TirePowerIntentType::BRAKE_OR_IDLE};
        }
    } else {
        auto clip_power = [&max_surface_power](float p){
            return signed_min(p, max_surface_power);
        };
        float sp = clip_power(clip_power(engine_power_intent_.surface_power) + engine_power_intent_.delta_power);
        return TirePowerIntent{
            .power = sp / float(ntires_old_),
            .relaxation = engine_power_intent_.relaxation,
            .type = TirePowerIntentType::ACCELERATE_OR_BREAK};
    }
}

float RigidBodyEngine::surface_power() const {
    return engine_power_intent_.surface_power;
}

void RigidBodyEngine::set_surface_power(const EnginePowerIntent& engine_power_intent) {
    engine_power_intent_ = engine_power_intent;
}

void RigidBodyEngine::advance_time(float dt, const FixedArray<double, 3>& position) {
    if (!std::isnan(w_)) {
        engine_power_.auto_set_gear(w_);
    }
    if (audio_ != nullptr) {
        if ((engine_state_ == EngineState::OFF) || hand_brake_pulled_ || std::isnan(w_)) {
            audio_->notify_off();
        } else if (engine_state_ == EngineState::IDLE) {
            audio_->notify_idle(engine_power_.engine_w(w_));
        } else if (engine_state_ == EngineState::ACCELERATE) {
            audio_->notify_driving(engine_power_.engine_w(w_));
        }
        audio_->set_position(position.casted<float>());
    }
}

void RigidBodyEngine::notify_off() {
    engine_state_ = EngineState::OFF;
    w_ = 0.f;
}

void RigidBodyEngine::notify_idle(float w) {
    engine_state_ = EngineState::IDLE;
    w_ = w;
}

void RigidBodyEngine::notify_accelerate(float w) {
    engine_state_ = EngineState::ACCELERATE;
    w_ = w;
}

std::ostream& Mlib::operator << (std::ostream& ostr, const TirePowerIntent& power_intent) {
    ostr << "Power intent\n";
    ostr << "   intent type " << (int)power_intent.type << std::endl;
    ostr << "   power " << power_intent.power << std::endl;
    return ostr;
}

std::ostream& Mlib::operator << (std::ostream& ostr, const RigidBodyEngine& engine) {
    ostr << "Engine\n";
    ostr << "   engine_state " << (int)engine.engine_state_ << '\n';
    ostr << "   w " << engine.w_ << '\n';
    ostr << "   surface_power " << engine.engine_power_intent_.surface_power << '\n';
    ostr << "   delta_power " << engine.engine_power_intent_.delta_power << '\n';
    for (size_t c : engine.tires_consumed_) {
        ostr << "   consumed " << c << '\n';
    }
    ostr << engine.engine_power_ << '\n';
    ostr << "   ntires_old " << engine.ntires_old_ << '\n';
    ostr << "   hand_brake_pulled " << (int)engine.hand_brake_pulled_ << '\n';
    return ostr;
}
