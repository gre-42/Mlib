#include "Human_As_Car_Controller.hpp"
#include <Mlib/Math/Signed_Min.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Steering_Type.hpp>
#include <Mlib/Scene_Graph/Style_Updater.hpp>
#include <stdexcept>

using namespace Mlib;

HumanAsCarController::HumanAsCarController(
    RigidBodyVehicle* rb,
    float angular_velocity,
    float steering_multiplier)
: RigidBodyVehicleController{ rb, SteeringType::CAR },
  angular_velocity_{ angular_velocity },
  steering_multiplier_{ steering_multiplier }
{}

HumanAsCarController::~HumanAsCarController()
{}

void HumanAsCarController::apply() {
    rb_->set_surface_power("legs", surface_power_); // NAN=break
    if (!std::isnan(steer_angle_)) {
        rb_->rbi_.rbp_.w_(1) = signed_min(steer_angle_ * steering_multiplier_, angular_velocity_);
    }
    if (rb_->style_updater_ != nullptr) {
        rb_->style_updater_->notify_movement_intent();
    }
}
