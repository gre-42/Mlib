#include "Tank_Controller.hpp"
#include <Mlib/Physics/Misc/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Misc/Steering_Type.hpp>
#include <Mlib/Scene_Graph/Style_Updater.hpp>
#include <stdexcept>

using namespace Mlib;

TankController::TankController(
    RigidBodyVehicle* rb,
    const std::vector<size_t>& left_tires,
    const std::vector<size_t>& right_tires,
    float steering_multiplier)
: RigidBodyVehicleController{ rb, SteeringType::TANK },
  left_tires_{ left_tires },
  right_tires_{ right_tires },
  steering_multiplier_{ steering_multiplier }
{}

TankController::~TankController()
{}

void TankController::apply() {
    if (std::isnan(surface_power_)) {
        rb_->set_surface_power("left", NAN);
        rb_->set_surface_power("right", NAN);
    } else {
        rb_->set_surface_power("left",  surface_power_, steer_angle_ == 0 ? 0.f : -steer_angle_ * steering_multiplier_);
        rb_->set_surface_power("right", surface_power_, steer_angle_ == 0 ? 0.f : +steer_angle_ * steering_multiplier_);
    }
    if (rb_->style_updater_ != nullptr) {
        rb_->style_updater_->notify_movement_intent();
    }
}
