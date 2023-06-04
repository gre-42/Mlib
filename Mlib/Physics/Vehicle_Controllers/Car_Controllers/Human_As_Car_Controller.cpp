#include "Human_As_Car_Controller.hpp"
#include <Mlib/Math/Signed_Min.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Steering_Type.hpp>
#include <Mlib/Scene_Graph/Animation/Animation_State_Updater.hpp>
#include <stdexcept>

using namespace Mlib;

HumanAsCarController::HumanAsCarController(
    RigidBodyVehicle& rb,
    YawPitchLookAtNodes& ypln,
    float steering_multiplier)
: RigidBodyVehicleController{ rb, SteeringType::CAR },
  steering_multiplier_{ steering_multiplier },
  ypln_{ ypln }
{}

HumanAsCarController::~HumanAsCarController()
{}

void HumanAsCarController::apply() {
    rb_.set_surface_power("legs", EnginePowerIntent{.surface_power = surface_power_}); // NAN=break
    if (!std::isnan(steer_angle_)) {
        ypln_.increment_yaw(steer_angle_ * steer_relaxation_ * steering_multiplier_);
    }
    if (rb_.animation_state_updater_ != nullptr) {
        rb_.animation_state_updater_->notify_movement_intent();
    }
    RigidBodyVehicleController::apply();
}
