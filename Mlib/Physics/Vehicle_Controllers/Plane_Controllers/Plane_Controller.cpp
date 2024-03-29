#include "Plane_Controller.hpp"
#include <Mlib/Math/Signed_Min.hpp>
#include <Mlib/Physics/Actuators/Engine_Power_Intent.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Steering_Type.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Vehicle_Domain.hpp>
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
    std::vector<size_t> right_flap_wing_ids,
    std::map<size_t, float> tire_angles,
    float yaw_amount_to_tire_angle,
    VehicleDomain vehicle_domain)
: RigidBodyPlaneController{ rb, SteeringType::CAR },
  tire_angles_{ std::move(tire_angles) },
  yaw_amount_to_tire_angle_{yaw_amount_to_tire_angle},
  left_front_aileron_wing_ids_{std::move(left_front_aileron_wing_ids)},
  right_front_aileron_wing_ids_{std::move(right_front_aileron_wing_ids)},
  left_rear_aileron_wing_ids_{std::move(left_rear_aileron_wing_ids)},
  right_rear_aileron_wing_ids_{std::move(right_rear_aileron_wing_ids)},
  left_rudder_wing_ids_{std::move(left_rudder_wing_ids)},
  right_rudder_wing_ids_{std::move(right_rudder_wing_ids)},
  left_flap_wing_ids_{std::move(left_flap_wing_ids)},
  right_flap_wing_ids_{std::move(right_flap_wing_ids)},
  vehicle_domain_{ vehicle_domain }
{}

PlaneController::~PlaneController() = default;

void PlaneController::apply() {
    if (vehicle_domain_ == VehicleDomain::AIR) {
        rb_.set_surface_power("wheels", EnginePowerIntent{.surface_power = 0.f});  // 0=idle
        rb_.set_surface_power("turbine", EnginePowerIntent{
            .surface_power = turbine_power_,
            .drive_relaxation = throttle_relaxation_});
        for (const auto& [tire_id, _] : tire_angles_) {
            rb_.set_tire_angle_y(tire_id, 0.f);
        }
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
            rb_.set_wing_angle_of_attack(i, -yaw_amount_ * yaw_relaxation_);
        }
        for (size_t i : left_flap_wing_ids_) {
            rb_.set_wing_brake_angle(i, brake_amount_ * throttle_relaxation_);
        }
        for (size_t i : right_flap_wing_ids_) {
            rb_.set_wing_brake_angle(i, brake_amount_ * throttle_relaxation_);
        }
    } else if (vehicle_domain_ == VehicleDomain::GROUND) {
        rb_.set_surface_power("wheels", EnginePowerIntent{.surface_power = 0.f});  // 0=idle
        for (const auto& [tire_id, tire] : tire_angles_) {
            float ang = signed_min(yaw_amount_ * yaw_amount_to_tire_angle_, tire);
            rb_.set_tire_angle_y(tire_id, ang);
        }
    } else {
        THROW_OR_ABORT("Unknown vehicle domain");
    }
    if (rb_.animation_state_updater_ != nullptr) {
        rb_.animation_state_updater_->notify_movement_intent();
    }
}
