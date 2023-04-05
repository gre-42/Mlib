#include "Rigid_Body_Engine.hpp"
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Signed_Min.hpp>
#include <Mlib/Physics/Actuators/Engine_Event_Listener.hpp>
#include <Mlib/Physics/Units.hpp>
#include <cmath>

using namespace Mlib;

RigidBodyEngine::RigidBodyEngine(
    const EnginePower& engine_power,
    bool hand_brake_pulled,
    const std::shared_ptr<EngineEventListener>& audio)
: engine_power_intent_{
    .surface_power = 0.f,
    .drive_relaxation = 0.f,
    .delta_power = 0.f,
    .delta_relaxation = 0.f},
  engine_power_{ engine_power },
  ntires_old_{ 0 },
  hand_brake_pulled_{ hand_brake_pulled },
  audio_{ audio }
{}

RigidBodyEngine::~RigidBodyEngine()
{}

void RigidBodyEngine::write_status(std::ostream& ostr, StatusComponents status_components) const {
    THROW_OR_ABORT("Unsupported status component: " + std::to_string((unsigned int)status_components));
}

float RigidBodyEngine::get_value(StatusComponents status_components) const {
    if (status_components == StatusComponents::ABS_ANGULAR_VELOCITY) {
        return std::abs(engine_w() / rpm);
    }
    THROW_OR_ABORT("Unsupported status component: " + std::to_string((unsigned int)status_components));
}

StatusWriter& RigidBodyEngine::child_status_writer(const std::vector<std::string>& name) {
    THROW_OR_ABORT("RigidBodyEngine has no children");
}

void RigidBodyEngine::reset_forces() {
    ntires_old_ = tires_consumed_.size();
    tires_consumed_.clear();
    tires_w_.clear();
}

TirePowerIntent RigidBodyEngine::consume_abs_surface_power(size_t tire_id, const float* tire_w) {
    tires_w_.insert(tire_w);
    if (!tires_consumed_.insert(tire_id).second ||
        (tires_consumed_.size() > ntires_old_)) {
        return TirePowerIntent{
            .power = 0,
            .relaxation = 1.f,
            .type = TirePowerIntentType::ALWAYS_IDLE};
    }

    float max_surface_power = (ntires_old_ == 0)
            ? 0.f
            : engine_power_.get_power();
    if (hand_brake_pulled_ || std::isnan(engine_power_intent_.surface_power) || std::isnan(max_surface_power)) {
        return TirePowerIntent{
            .power = NAN,
            .relaxation = engine_power_intent_.drive_relaxation,
            .type = TirePowerIntentType::ALWAYS_BRAKE};
    } else {
        auto clip_power = [&max_surface_power](float p){
            return signed_min(p, max_surface_power);
        };
        float sp = 
            clip_power(engine_power_intent_.surface_power) +
            engine_power_intent_.delta_power * cubed(engine_power_intent_.delta_relaxation);
        float relaxation = std::max(engine_power_intent_.drive_relaxation, engine_power_intent_.delta_relaxation);
        if (max_surface_power == 0) {
            return TirePowerIntent{
                .power = sign(sp),
                .relaxation = relaxation,
                .type = TirePowerIntentType::BRAKE_OR_IDLE};
        } else {
            return TirePowerIntent{
                .power = clip_power(sp) / float(ntires_old_),
                .relaxation = relaxation,
                .type = TirePowerIntentType::ACCELERATE_OR_BRAKE};
        }
    }
}

float RigidBodyEngine::surface_power() const {
    return engine_power_intent_.surface_power;
}

void RigidBodyEngine::set_surface_power(const EnginePowerIntent& engine_power_intent) {
    engine_power_intent_ = engine_power_intent;
}

void RigidBodyEngine::advance_time(float dt, const FixedArray<double, 3>& position) {
    float average_tire_w_;
    if (tires_w_.empty()) {
        average_tire_w_ = NAN;
    } else {
        average_tire_w_ = 0.f;
        for (const float* tire_w : tires_w_) {
            average_tire_w_ += *tire_w;
        }
        average_tire_w_ /= (float)tires_w_.size();
    }
    if (!std::isnan(average_tire_w_)) {
        engine_power_.auto_set_gear(dt, average_tire_w_);
    }
    if (audio_ != nullptr) {
        audio_->notify_rotation(
            engine_power_.engine_w(),
            average_tire_w_,
            engine_power_intent_,
            engine_power_.get_power());
        audio_->set_position(position);
    }
}

float RigidBodyEngine::engine_w() const {
    return engine_power_.engine_w();
}

std::ostream& Mlib::operator << (std::ostream& ostr, const TirePowerIntent& power_intent) {
    ostr << "Power intent\n";
    ostr << "   intent type " << (int)power_intent.type << std::endl;
    ostr << "   power " << power_intent.power << std::endl;
    return ostr;
}

std::ostream& Mlib::operator << (std::ostream& ostr, const RigidBodyEngine& engine) {
    ostr << "Engine\n";
    ostr << "   engine_power_intent " << engine.engine_power_intent_ << '\n';
    for (const float* tire_w : engine.tires_w_) {
        ostr << "   tire_w " << *tire_w << '\n';
    }
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
