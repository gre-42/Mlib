#include "Rigid_Body_Vehicle_Controller.hpp"
#include <cmath>

using namespace Mlib;

RigidBodyVehicleController::RigidBodyVehicleController(
    RigidBodyVehicle* rb,
    SteeringType steering_type)
: steering_type{ steering_type },
  rb_{ rb },
  steer_angle_{ NAN },
  surface_power_{ NAN },
  target_height_{ NAN }
{}

RigidBodyVehicleController::~RigidBodyVehicleController()
{}

void RigidBodyVehicleController::step_on_brakes() {
    surface_power_ = NAN;
}

void RigidBodyVehicleController::drive(float surface_power) {
    surface_power_ = surface_power;
}

void RigidBodyVehicleController::roll_tires() {
    surface_power_ = 0;
}

void RigidBodyVehicleController::steer(float angle) {
    steer_angle_ = angle;
}

void RigidBodyVehicleController::ascend_to(float target_height) {
    target_height_ = target_height;
}

void RigidBodyVehicleController::ascend_by(float delta_height) {
    if (!std::isnan(target_height_)) {
        target_height_ += delta_height;
    }
}

void RigidBodyVehicleController::reset(
    float surface_power,
    float steer_angle)
{
    surface_power_ = surface_power;
    steer_angle_ = steer_angle;
}
