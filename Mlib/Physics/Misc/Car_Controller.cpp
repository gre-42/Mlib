#include "Car_Controller.hpp"
#include <Mlib/Physics/Misc/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Misc/Steering_Type.hpp>
#include <Mlib/Scene_Graph/Style_Updater.hpp>
#include <stdexcept>

using namespace Mlib;

CarController::CarController(
    RigidBodyVehicle* rb,
    const std::map<size_t, float>& tire_angles)
: RigidBodyVehicleController{ rb, SteeringType::CAR },
  tire_angles_{ tire_angles }
{}

CarController::~CarController()
{}

void CarController::apply() {
    rb_->set_surface_power("main", surface_power_); // NAN=break
    if (std::isnan(surface_power_)) {
        rb_->set_surface_power("breaks", NAN);      // NAN=break
    } else {
        rb_->set_surface_power("breaks", 0);
    }
    for (const auto& x : tire_angles_) {
        float ang = sign(steer_angle_) * std::min(std::abs(steer_angle_), x.second);
        rb_->set_tire_angle_y(x.first, ang);
    }
    if (rb_->style_updater_ != nullptr) {
        rb_->style_updater_->notify_movement_intent();
    }
}
