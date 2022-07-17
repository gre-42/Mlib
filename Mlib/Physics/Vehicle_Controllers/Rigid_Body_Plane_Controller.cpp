#include "Rigid_Body_Plane_Controller.hpp"
#include <cmath>

using namespace Mlib;

RigidBodyPlaneController::RigidBodyPlaneController(
    RigidBodyVehicle* rb,
    SteeringType steering_type)
: steering_type{ steering_type },
  rb_{ rb },
  turbine_power_{ NAN },
  brake_amount_{ NAN },
  pitch_amount_{ NAN },
  yaw_amount_{ NAN },
  roll_amount_{ NAN }
{}

RigidBodyPlaneController::~RigidBodyPlaneController()
{}

void RigidBodyPlaneController::brake(float amount) {
    brake_amount_ = amount;
}

void RigidBodyPlaneController::accelerate(float turbine_power) {
    turbine_power_ = turbine_power;
}

void RigidBodyPlaneController::pitch(float amount) {
    pitch_amount_ = amount;
}

void RigidBodyPlaneController::yaw(float amount) {
    yaw_amount_ = amount;
}

void RigidBodyPlaneController::roll(float amount) {
    roll_amount_ = amount;
}

void RigidBodyPlaneController::reset(
    float turbine_power,
    float brake_amount,
    float pitch_amount,
    float yaw_amount,
    float roll_amount)
{
    turbine_power_ = turbine_power;
    brake_amount_ = brake_amount;
    pitch_amount_ = pitch_amount;
    yaw_amount_ = yaw_amount;
    roll_amount_ = roll_amount;
}
