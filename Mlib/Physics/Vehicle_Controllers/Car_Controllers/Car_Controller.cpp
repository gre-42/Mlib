#include "Car_Controller.hpp"
#include <Mlib/Math/Signed_Min.hpp>
#include <Mlib/Physics/Actuators/Engine_Power_Intent.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Steering_Type.hpp>
#include <Mlib/Scene_Graph/Animation/Animation_State_Updater.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

CarController::CarController(
    RigidBodyVehicle& rb,
    VariableAndHash<std::string> front_engine,
    VariableAndHash<std::string> rear_engine,
    std::vector<size_t> front_tire_ids,
    float max_tire_angle,
    Interp<float> tire_angle_interp,
    PhysicsEngine& physics_engine)
    : RigidBodyVehicleController{ rb, SteeringType::CAR }
    , front_engine_{ front_engine }
    , rear_engine_{ rear_engine }
    , front_tire_ids_{ std::move(front_tire_ids) }
    , max_tire_angle_{ max_tire_angle }
    , tire_angle_interp_{ std::move(tire_angle_interp) }
    , applied_{ false }
    , physics_engine_{ physics_engine }
{
    physics_engine_.add_controllable(*this);
}

CarController::~CarController()
{
    on_destroy.clear();
    physics_engine_.remove_controllable(*this);
}

void CarController::set_stearing_wheel_amount(float left_amount, float relaxation) {
    float v = std::abs(dot0d(
        rb_.rbp_.v_com_,
        rb_.rbp_.rotation_.column(2)));
    steer(left_amount * tire_angle_interp_(v), relaxation);
}

void CarController::apply() {
    if (applied_) {
        THROW_OR_ABORT("Car controller already applied");
    }
    applied_ = true;
    rb_.set_surface_power(front_engine_, EnginePowerIntent{
        .surface_power = surface_power_,
        .drive_relaxation = drive_relaxation_});   // NAN=break
    if (front_engine_ != rear_engine_) {
        rb_.set_surface_power(rear_engine_, EnginePowerIntent{
            .surface_power = surface_power_,
            .drive_relaxation = drive_relaxation_}); // NAN=break
    }
    if (!front_tire_ids_.empty()) {
        float ang = signed_min(steer_angle_ * steer_relaxation_, max_tire_angle_);
        for (size_t tire_id : front_tire_ids_) {
            rb_.set_tire_angle_y(tire_id, ang);
        }
    }
    if (rb_.animation_state_updater_ != nullptr) {
        rb_.animation_state_updater_->notify_movement_intent();
    }
    RigidBodyVehicleController::apply();
}

void CarController::notify_reset(const PhysicsEngineConfig& cfg, const PhysicsPhase& phase) {
    applied_ = false;
}
