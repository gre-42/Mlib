#include "Heli_Controller.hpp"
#include <Mlib/Math/Signed_Min.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Steering_Type.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Vehicle_Domain.hpp>
#include <Mlib/Scene_Graph/Style_Updater.hpp>
#include <stdexcept>

using namespace Mlib;

HeliController::HeliController(
    RigidBodyVehicle* rb,
    const std::map<size_t, float>& tire_angles,
    size_t main_rotor_id,
    size_t rear_rotor_id,
    float ascend_multiplier,
    float yaw_multiplier,
    VehicleDomain vehicle_domain)
: RigidBodyVehicleController{ rb, SteeringType::CAR },
  tire_angles_{ tire_angles },
  main_rotor_id_{ main_rotor_id },
  rear_rotor_id_{ rear_rotor_id },
  ascend_multiplier_{ ascend_multiplier },
  yaw_multiplier_{ yaw_multiplier },
  vehicle_domain_{ vehicle_domain }
{}

HeliController::~HeliController()
{}

void HeliController::apply() {
    if (vehicle_domain_ == VehicleDomain::AIR) {
        rb_->set_surface_power("wheels", NAN);  // NAN=break
        for (const auto& x : tire_angles_) {
            rb_->set_tire_angle_y(x.first, 0.f);
        }
        rb_->set_surface_power("main_rotor", std::isnan(ascend_velocity_) ? 0.f : ascend_multiplier_ * ascend_velocity_);
        rb_->set_rotor_angle_x(main_rotor_id_, std::isnan(surface_power_) ? 0.f : 0.2f * sign(surface_power_));
        float ang = signed_min(steer_angle_, 0.5f);
        rb_->set_surface_power("rear_rotor", yaw_multiplier_ * ang);
    } else if (vehicle_domain_ == VehicleDomain::GROUND) {
        rb_->set_surface_power("wheels", surface_power_);  // NAN=break
        for (const auto& x : tire_angles_) {
            float ang = signed_min(steer_angle_, x.second);
            rb_->set_tire_angle_y(x.first, ang);
        }
    } else {
        throw std::runtime_error("Unknown vehicle domain");
    }
    if (rb_->style_updater_ != nullptr) {
        rb_->style_updater_->notify_movement_intent();
    }
}
