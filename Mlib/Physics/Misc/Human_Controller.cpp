#include "Human_Controller.hpp"
#include <Mlib/Physics/Misc/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Misc/Steering_Type.hpp>
#include <Mlib/Scene_Graph/Style_Updater.hpp>
#include <stdexcept>

using namespace Mlib;

HumanController::HumanController(
    RigidBodyVehicle* rb,
    float angular_velocity,
    float steering_multiplier)
: RigidBodyVehicleController{ rb, SteeringType::CAR },
  angular_velocity_{ angular_velocity },
  steering_multiplier_{ steering_multiplier }
{}

HumanController::~HumanController()
{}

void HumanController::apply() {
    rb_->set_surface_power("legs", surface_power_); // NAN=break
    if (!std::isnan(angular_velocity_)) {
        rb_->rbi_.rbp_.w_(1) = sign(steer_angle_) * std::min(std::abs(steer_angle_ * steering_multiplier_), angular_velocity_);
    }
    if (rb_->style_updater_ != nullptr) {
        rb_->style_updater_->notify_movement_intent();
    }
}
