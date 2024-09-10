#include "Plane_Controller.hpp"
#include <Mlib/Math/Signed_Min.hpp>
#include <Mlib/Physics/Actuators/Engine_Power_Intent.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Steering_Type.hpp>
#include <Mlib/Scene_Graph/Animation/Animation_State_Updater.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

PlaneController::PlaneController(
    RigidBodyVehicle& rb,
    std::vector<size_t> left_front_aileron_wing_ids,
    std::vector<size_t> right_front_aileron_wing_ids,
    std::vector<size_t> left_rear_aileron_wing_ids,
    std::vector<size_t> right_rear_aileron_wing_ids,
    std::vector<size_t> left_rudder_wing_ids,
    std::vector<size_t> right_rudder_wing_ids,
    std::vector<size_t> left_flap_wing_ids,
    std::vector<size_t> right_flap_wing_ids)
    : RigidBodyPlaneController{ rb, SteeringType::CAR }
    , left_front_aileron_wing_ids_{ std::move(left_front_aileron_wing_ids) }
    , right_front_aileron_wing_ids_{ std::move(right_front_aileron_wing_ids) }
    , left_rear_aileron_wing_ids_{ std::move(left_rear_aileron_wing_ids) }
    , right_rear_aileron_wing_ids_{ std::move(right_rear_aileron_wing_ids) }
    , left_rudder_wing_ids_{ std::move(left_rudder_wing_ids) }
    , right_rudder_wing_ids_{ std::move(right_rudder_wing_ids) }
    , left_flap_wing_ids_{ std::move(left_flap_wing_ids) }
    , right_flap_wing_ids_{ std::move(right_flap_wing_ids) }
{}

PlaneController::~PlaneController() = default;

static const auto turbine_name = VariableAndHash<std::string>{ "turbine" };

void PlaneController::apply() {
    rb_.set_surface_power(turbine_name, EnginePowerIntent{
        .surface_power = turbine_power_,
        .drive_relaxation = throttle_relaxation_ });
    for (size_t i : left_front_aileron_wing_ids_) {
        rb_.set_wing_angle_of_attack(i, -pitch_amount_ * pitch_relaxation_ + roll_amount_ * roll_relaxation_);
    }
    for (size_t i : right_front_aileron_wing_ids_) {
        rb_.set_wing_angle_of_attack(i, -pitch_amount_ * pitch_relaxation_ - roll_amount_ * roll_relaxation_);
    }
    for (size_t i : left_rear_aileron_wing_ids_) {
        rb_.set_wing_angle_of_attack(i, pitch_amount_ * pitch_relaxation_ + roll_amount_ * roll_relaxation_);
    }
    for (size_t i : right_rear_aileron_wing_ids_) {
        rb_.set_wing_angle_of_attack(i, pitch_amount_ * pitch_relaxation_ - roll_amount_ * roll_relaxation_);
    }
    for (size_t i : left_rudder_wing_ids_) {
        rb_.set_wing_angle_of_attack(i, yaw_amount_ * yaw_relaxation_);
    }
    for (size_t i : right_rudder_wing_ids_) {
        rb_.set_wing_angle_of_attack(i, yaw_amount_ * yaw_relaxation_);
    }
    for (size_t i : left_flap_wing_ids_) {
        rb_.set_wing_brake_angle(i, brake_amount_ * throttle_relaxation_);
    }
    for (size_t i : right_flap_wing_ids_) {
        rb_.set_wing_brake_angle(i, brake_amount_ * throttle_relaxation_);
    }
    if (rb_.animation_state_updater_ != nullptr) {
        rb_.animation_state_updater_->notify_movement_intent();
    }
}
