#include "Heli_Controller.hpp"
#include <Mlib/Physics/Misc/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Misc/Steering_Type.hpp>
#include <Mlib/Scene_Graph/Style_Updater.hpp>
#include <stdexcept>

using namespace Mlib;

HeliController::HeliController(
    RigidBodyVehicle* rb,
    const std::map<size_t, float>& tire_angles)
: RigidBodyVehicleController{ rb, SteeringType::CAR },
  tire_angles_{ tire_angles }
{}

HeliController::~HeliController()
{}

void HeliController::apply() {
    rb_->set_surface_power("wheels", surface_power_);  // NAN=break
    for (const auto& x : tire_angles_) {
        float ang = sign(steer_angle_) * std::min(std::abs(steer_angle_), x.second);
        rb_->set_tire_angle_y(x.first, ang);
    }
    if (rb_->style_updater_ != nullptr) {
        rb_->style_updater_->notify_movement_intent();
    }
}
