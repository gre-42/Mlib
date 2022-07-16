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
    const std::vector<size_t>& front_pitch_wing_ids,
    const std::vector<size_t>& rear_pitch_wing_ids,
    const std::vector<size_t>& yaw_wing_ids,
    const std::vector<size_t>& left_roll_wing_ids,
    const std::vector<size_t>& right_roll_wing_ids,
    const std::map<size_t, float>& tire_angles,
    float yaw_amount_to_tire_angle,
    size_t turbine_id,
    VehicleDomain vehicle_domain)
: RigidBodyPlaneController{ rb, SteeringType::CAR },
  turbine_id_{ turbine_id },
  tire_angles_{ tire_angles },
  yaw_amount_to_tire_angle_{ yaw_amount_to_tire_angle },
  front_pitch_wing_ids_{front_pitch_wing_ids},
  rear_pitch_wing_ids_{rear_pitch_wing_ids},
  yaw_wing_ids_{yaw_wing_ids},
  left_roll_wing_ids_{left_roll_wing_ids},
  right_roll_wing_ids_{right_roll_wing_ids},
  vehicle_domain_{ vehicle_domain }
{}

PlaneController::~PlaneController()
{}

void PlaneController::apply() {
    if (vehicle_domain_ == VehicleDomain::AIR) {
        rb_->set_surface_power("wheels", 0.f);  // 0=idle
        rb_->set_surface_power("turbine", turbine_power_);
        for (const auto& [tire_id, _] : tire_angles_) {
            rb_->set_tire_angle_y(tire_id, 0.f);
        }
        for (size_t i : front_pitch_wing_ids_) {
            rb_->set_wing_angle(i, -pitch_amount_);
        }
        for (size_t i : rear_pitch_wing_ids_) {
            rb_->set_wing_angle(i, pitch_amount_);
        }
        for (size_t i : yaw_wing_ids_) {
            rb_->set_wing_angle(i, yaw_amount_);
        }
        for (size_t i : left_roll_wing_ids_) {
            rb_->set_wing_angle(i, roll_amount_);
        }
        for (size_t i : right_roll_wing_ids_) {
            rb_->set_wing_angle(i, -roll_amount_);
        }
    } else if (vehicle_domain_ == VehicleDomain::GROUND) {
        rb_->set_surface_power("wheels", 0.f);  // 0=idle
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
