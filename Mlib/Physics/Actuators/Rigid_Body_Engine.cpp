#include "Rigid_Body_Engine.hpp"
#include <Mlib/Audio/Audio_Entity_State.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Signed_Min.hpp>
#include <Mlib/Physics/Actuators/Engine_Event_Listeners.hpp>
#include <Mlib/Physics/Actuators/Engine_Power_Delta_Intent.hpp>
#include <Mlib/Physics/Actuators/IEngine_Event_Listener.hpp>
#include <Mlib/Physics/Actuators/Tire_Power_Intent.hpp>
#include <Mlib/Physics/Actuators/Velocity_Classification.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Phase.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Time_Step.hpp>
#include <Mlib/Physics/Units.hpp>
#include <cmath>

using namespace Mlib;

RigidBodyEngine::RigidBodyEngine(
    const std::optional<EnginePower>& engine_power,
    bool hand_brake_pulled,
    std::shared_ptr<IEngineEventListener> listener)
    : engine_power_intent_{
        .surface_power = 0.f,
        .drive_relaxation = 0.f}
    , engine_power_{ engine_power }
    , ntires_old_{ 0 }
    , hand_brake_pulled_{ hand_brake_pulled }
    , listener_{ std::move(listener) }
{
    if (!engine_power.has_value() && (listener_ != nullptr)) {
        THROW_OR_ABORT("Engine listener is set but no engine was provided");
    }
}

RigidBodyEngine::~RigidBodyEngine() = default;

void RigidBodyEngine::write_status(std::ostream& ostr, StatusComponents status_components, const StaticWorld& world) const {
    THROW_OR_ABORT("Unsupported status component: " + std::to_string((unsigned int)status_components));
}

float RigidBodyEngine::get_value(StatusComponents status_components) const {
    if (status_components == StatusComponents::ABS_ANGULAR_VELOCITY) {
        return std::abs(engine_w() / rpm);
    }
    THROW_OR_ABORT("Unsupported status component: " + std::to_string((unsigned int)status_components));
}

StatusWriter& RigidBodyEngine::child_status_writer(const std::vector<VariableAndHash<std::string>>& name) {
    THROW_OR_ABORT("RigidBodyEngine has no children");
}

void RigidBodyEngine::reset_forces() {
    ntires_old_ = tires_consumed_.size();
    tires_consumed_.clear();
    tires_w_.clear();
}

TirePowerIntent RigidBodyEngine::consume_tire_power(
    size_t tire_id,
    const float* tire_w,
    const EnginePowerDeltaIntent& delta_intent,
    VelocityClassification velocity_classification)
{
    tires_w_.insert(tire_w);
    if (!tires_consumed_.insert(tire_id).second ||
        (tires_consumed_.size() > ntires_old_)) {
        return TirePowerIntent{
            .power = 0,
            .relaxation = 1.f,
            .type = TirePowerIntentType::IDLE};
    }

    float max_surface_power = (ntires_old_ == 0) || !engine_power_.has_value()
        ? 0.f
        : engine_power_->get_power();
    if (hand_brake_pulled_ || std::isnan(max_surface_power)) {
        return TirePowerIntent{
            .power = NAN,
            .relaxation = 1.f,
            .type = TirePowerIntentType::BRAKE};
    } else if (std::isnan(engine_power_intent_.surface_power)) {
        return TirePowerIntent{
            .power = NAN,
            .relaxation = engine_power_intent_.drive_relaxation,
            .type = TirePowerIntentType::BRAKE};
    } else {
        float max_relaxation = std::max(engine_power_intent_.drive_relaxation, delta_intent.delta_relaxation);
        float sum_relaxation = engine_power_intent_.drive_relaxation + delta_intent.delta_relaxation;
        if ((max_surface_power == 0.f) ||
            (sum_relaxation < 1e-12) ||
            ((velocity_classification == VelocityClassification::FAST) &&
             (sign(engine_power_intent_.surface_power) == sign(*tire_w))))
        {
            float sp =
                sign(engine_power_intent_.surface_power) * engine_power_intent_.drive_relaxation +
                2.f * sign(delta_intent.delta_power) * cubed(delta_intent.delta_relaxation);
            return TirePowerIntent{
                .power = sign(sp),
                .relaxation = max_relaxation,
                .type = TirePowerIntentType::BRAKE_OR_IDLE};
        } else {
            auto clip_power = [&max_surface_power](float p){
                return signed_min(p, max_surface_power);
            };
            float sp =
                clip_power(engine_power_intent_.surface_power) * engine_power_intent_.drive_relaxation +
                delta_intent.delta_power * delta_intent.delta_relaxation;
            if (engine_power_intent_.drive_relaxation > 1e-12) {
                sp /= engine_power_intent_.drive_relaxation;
            }
            if ((velocity_classification == VelocityClassification::FAST) &&
                (sign(sp) == sign(*tire_w)))
            {
                return TirePowerIntent{
                    .power = clip_power(sp) / float(ntires_old_),
                    .relaxation = 0.5f * cubed(delta_intent.delta_relaxation) / sum_relaxation,
                    .type = TirePowerIntentType::BRAKE_OR_IDLE};
            } else {
                return TirePowerIntent{
                    .power = clip_power(sp) / float(ntires_old_),
                    .relaxation = max_relaxation,
                    .type = TirePowerIntentType::ACCELERATE};
            }
        }
    }
}

TirePowerIntent RigidBodyEngine::consume_rotor_power(
    size_t rotor_id,
    const float* rotor_w,
    const EnginePowerDeltaIntent& delta_intent)
{
    tires_w_.insert(rotor_w);
    if (!tires_consumed_.insert(rotor_id).second ||
        (tires_consumed_.size() > ntires_old_)) {
        return TirePowerIntent{
            .power = 0,
            .relaxation = 1.f,
            .type = TirePowerIntentType::IDLE};
    }

    float max_surface_power = (ntires_old_ == 0) || !engine_power_.has_value()
        ? 0.f
        : engine_power_->get_power();
    if (hand_brake_pulled_ || std::isnan(max_surface_power)) {
        return TirePowerIntent{
            .power = NAN,
            .relaxation = 1.f,
            .type = TirePowerIntentType::BRAKE};
    } else if (std::isnan(engine_power_intent_.surface_power)) {
        return TirePowerIntent{
            .power = NAN,
            .relaxation = engine_power_intent_.drive_relaxation,
            .type = TirePowerIntentType::BRAKE};
    } else {
        float max_relaxation = std::max(engine_power_intent_.drive_relaxation, delta_intent.delta_relaxation);
        auto clip_power = [&max_surface_power](float p){
            return signed_min(p, max_surface_power);
        };
        float sp =
            clip_power(engine_power_intent_.surface_power) * engine_power_intent_.drive_relaxation +
            delta_intent.delta_power * delta_intent.delta_relaxation;
        if (engine_power_intent_.drive_relaxation > 1e-12) {
            sp /= engine_power_intent_.drive_relaxation;
        }
        return TirePowerIntent{
            .power = clip_power(sp) / float(ntires_old_),
            .relaxation = max_relaxation,
            .type = TirePowerIntentType::ACCELERATE};
    }
}

float RigidBodyEngine::surface_power() const {
    return engine_power_intent_.surface_power;
}

void RigidBodyEngine::set_surface_power(const EnginePowerIntent& engine_power_intent) {
    engine_power_intent_ = engine_power_intent;
}

void RigidBodyEngine::advance_time(
    const PhysicsTimeStep& dt,
    const PhysicsPhase& phase,
    const RotatingFrame<SceneDir, ScenePos, 3>& frame,
    const StaticWorld& static_world)
{
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
    if (engine_power_.has_value()) {
        if (!std::isnan(average_tire_w_)) {
            engine_power_->auto_set_gear(dt.dt_substep, average_tire_w_);
        }
        if (!phase.burn_in && (phase.substep == 0) && (listener_ != nullptr)) {
            listener_->notify_rotation(
                engine_power_->engine_w(),
                average_tire_w_,
                engine_power_intent_,
                engine_power_->get_power(),
                static_world);
        }
    }
    if (!phase.burn_in && (phase.substep == 0) && (listener_ != nullptr)) {
        listener_->set_location(frame);
        listener_->advance_time(dt.dt_step);
    }
}

float RigidBodyEngine::engine_w() const {
    if (!engine_power_.has_value()) {
        THROW_OR_ABORT("RigidBodyEngine::engine_w() requested but no engine provided");
    }
    return engine_power_->engine_w();
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
    for (size_t c : engine.tires_consumed_) {
        ostr << "   consumed " << c << '\n';
    }
    if (engine.engine_power_.has_value()) {
        ostr << *engine.engine_power_ << '\n';
    }
    ostr << "   ntires_old " << engine.ntires_old_ << '\n';
    ostr << "   hand_brake_pulled " << (int)engine.hand_brake_pulled_ << '\n';
    return ostr;
}
