#include "Rigid_Body_Vehicle_Controller.hpp"
#include <cmath>

using namespace Mlib;

RigidBodyVehicleController::RigidBodyVehicleController(
    RigidBodyVehicle* rb,
    SteeringType steering_type)
: steering_type{ steering_type },
  rb_{ rb },
  surface_power_{ NAN },
  drive_relaxation_{ 0.f },
  steer_angle_{ NAN },
  steer_relaxation_{ 0.f },
  target_height_{ NAN }
{}

RigidBodyVehicleController::~RigidBodyVehicleController()
{}

void RigidBodyVehicleController::step_on_brakes() {
    surface_power_ = NAN;
}

void RigidBodyVehicleController::drive(float surface_power, float relaxation) {
    if (relaxation >= drive_relaxation_) {
        surface_power_ = surface_power;
        drive_relaxation_ = relaxation;
    }
}

void RigidBodyVehicleController::roll_tires() {
    surface_power_ = 0;
}

void RigidBodyVehicleController::steer(float angle, float relaxation) {
    if (relaxation >= steer_relaxation_) {
        steer_angle_ = angle;
        steer_relaxation_ = relaxation;
    }
}

void RigidBodyVehicleController::ascend_to(double target_height) {
    target_height_ = target_height;
}

void RigidBodyVehicleController::ascend_by(double delta_height) {
    if (!std::isnan(target_height_)) {
        target_height_ += delta_height;
    }
}

void RigidBodyVehicleController::reset_parameters(
    float surface_power,
    float steer_angle)
{
    surface_power_ = surface_power;
    steer_angle_ = steer_angle;
}

void RigidBodyVehicleController::reset_relaxation(
    float drive_relaxation,
    float steer_relaxation)
{
    drive_relaxation_ = drive_relaxation;
    steer_relaxation_ = steer_relaxation;
}
