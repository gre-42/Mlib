#include "Rigid_Body_Plane_Controller.hpp"
#include <cmath>

using namespace Mlib;

RigidBodyPlaneController::RigidBodyPlaneController(
    RigidBodyVehicle* rb,
    SteeringType steering_type)
: steering_type{ steering_type },
  rb_{ rb },
  turbine_power_{ NAN },
  flaps_angle_{ NAN },
  pitch_amount_{ NAN },
  yaw_amount_{ NAN },
  roll_amount_{ NAN }
{}

RigidBodyPlaneController::~RigidBodyPlaneController()
{}

void RigidBodyPlaneController::open_flaps(float angle) {
    flaps_angle_ = angle;
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
    float flaps_angle,
    float pitch_amount,
    float yaw_amount,
    float roll_amount)
{
    turbine_power_ = turbine_power;
    flaps_angle_ = flaps_angle;
    pitch_amount_ = pitch_amount;
    yaw_amount_ = yaw_amount;
    roll_amount_ = roll_amount;
}
