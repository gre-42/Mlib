#include "Plane_Controller.hpp"
#include <Mlib/Math/Signed_Min.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Steering_Type.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Vehicle_Domain.hpp>
#include <Mlib/Scene_Graph/Animation_State_Updater.hpp>
#include <stdexcept>

using namespace Mlib;

PlaneController::PlaneController(
    RigidBodyVehicle* rb,
    const std::vector<size_t>& left_front_aileron_wing_ids,
    const std::vector<size_t>& right_front_aileron_wing_ids,
    const std::vector<size_t>& left_rear_aileron_wing_ids,
    const std::vector<size_t>& right_rear_aileron_wing_ids,
    const std::vector<size_t>& left_rudder_wing_ids,
    const std::vector<size_t>& right_rudder_wing_ids,
    const std::vector<size_t>& left_flap_wing_ids,
    const std::vector<size_t>& right_flap_wing_ids,
    const std::map<size_t, float>& tire_angles,
    float yaw_amount_to_tire_angle,
    size_t turbine_id,
    VehicleDomain vehicle_domain)
: RigidBodyPlaneController{ rb, SteeringType::CAR },
  turbine_id_{ turbine_id },
  tire_angles_{ tire_angles },
  yaw_amount_to_tire_angle_{ yaw_amount_to_tire_angle },
  left_front_aileron_wing_ids_{left_front_aileron_wing_ids},
  right_front_aileron_wing_ids_{right_front_aileron_wing_ids},
  left_rear_aileron_wing_ids_{left_rear_aileron_wing_ids},
  right_rear_aileron_wing_ids_{right_rear_aileron_wing_ids},
  left_rudder_wing_ids_{left_rudder_wing_ids},
  right_rudder_wing_ids_{right_rudder_wing_ids},
  left_flap_wing_ids_{left_flap_wing_ids},
  right_flap_wing_ids_{right_flap_wing_ids},
  vehicle_domain_{ vehicle_domain }
{}

PlaneController::~PlaneController()
{}

void PlaneController::apply() {
    if (vehicle_domain_ == VehicleDomain::AIR) {
        rb_->set_surface_power("wheels", EnginePowerIntent{.surface_power = 0.f});  // 0=idle
        rb_->set_surface_power("turbine", EnginePowerIntent{.surface_power = turbine_power_});
        for (const auto& [tire_id, _] : tire_angles_) {
            rb_->set_tire_angle_y(tire_id, 0.f);
        }
        for (size_t i : left_front_aileron_wing_ids_) {
            rb_->set_wing_angle_of_attack(i, -pitch_amount_ + roll_amount_);
        }
        for (size_t i : right_front_aileron_wing_ids_) {
            rb_->set_wing_angle_of_attack(i, -pitch_amount_ - roll_amount_);
        }
        for (size_t i : left_rear_aileron_wing_ids_) {
            rb_->set_wing_angle_of_attack(i, pitch_amount_ + roll_amount_);
        }
        for (size_t i : right_rear_aileron_wing_ids_) {
            rb_->set_wing_angle_of_attack(i, pitch_amount_ - roll_amount_);
        }
        for (size_t i : left_rudder_wing_ids_) {
            rb_->set_wing_angle_of_attack(i, yaw_amount_);
        }
        for (size_t i : right_rudder_wing_ids_) {
            rb_->set_wing_angle_of_attack(i, -yaw_amount_);
        }
        for (size_t i : left_flap_wing_ids_) {
            rb_->set_wing_brake_angle(i, brake_amount_);
        }
        for (size_t i : right_flap_wing_ids_) {
            rb_->set_wing_brake_angle(i, brake_amount_);
        }
    } else if (vehicle_domain_ == VehicleDomain::GROUND) {
        rb_->set_surface_power("wheels", EnginePowerIntent{.surface_power = 0.f});  // 0=idle
        for (const auto& [tire_id, tire] : tire_angles_) {
            float ang = signed_min(yaw_amount_ * yaw_amount_to_tire_angle_, tire);
            rb_->set_tire_angle_y(tire_id, ang);
        }
    } else {
        throw std::runtime_error("Unknown vehicle domain");
    }
    if (rb_->animation_state_updater_ != nullptr) {
        rb_->animation_state_updater_->notify_movement_intent();
    }
}
