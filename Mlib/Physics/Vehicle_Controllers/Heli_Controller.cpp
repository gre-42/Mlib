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
    FixedArray<float, 3> angle_multipliers,
    const PidController<float, float>& height_pid,
    VehicleDomain vehicle_domain)
: RigidBodyVehicleController{ rb, SteeringType::CAR },
  height_pid_{ height_pid },
  tire_angles_{ tire_angles },
  main_rotor_id_{ main_rotor_id },
  rear_rotor_id_{ rear_rotor_id },
  angle_multipliers_{ angle_multipliers },
  vehicle_domain_{ vehicle_domain }
{
    ascend_to(rb->rbi_.abs_position()(1));
}

HeliController::~HeliController()
{}

static const size_t PITCH = 0;
static const size_t YAW = 1;
static const size_t ROLL = 2;

void HeliController::apply() {
    if (vehicle_domain_ == VehicleDomain::AIR) {
        rb_->set_surface_power("wheels", NAN);  // NAN=break
        for (const auto& x : tire_angles_) {
            rb_->set_tire_angle_y(x.first, 0.f);
        }
        rb_->set_surface_power(
            "main_rotor",
            std::isnan(target_height_)
                ? 0.f
                : std::min(0.f, height_pid_(rb_->rbi_.abs_position()(1) - target_height_)));
        rb_->set_rotor_angle_x(main_rotor_id_, std::isnan(surface_power_)
            ? 0.f
            : -angle_multipliers_(PITCH) * sign(surface_power_));
        float ang = signed_min(steer_angle_, 45 * float(M_PI / 180.f));
        rb_->set_rotor_angle_y(main_rotor_id_, angle_multipliers_(ROLL) * ang);
        rb_->set_surface_power("rear_rotor", angle_multipliers_(YAW) * ang);
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
