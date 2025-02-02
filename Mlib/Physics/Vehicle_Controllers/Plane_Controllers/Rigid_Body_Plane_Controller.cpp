#include "Rigid_Body_Plane_Controller.hpp"
#include <cmath>

using namespace Mlib;

RigidBodyPlaneController::RigidBodyPlaneController(
    RigidBodyVehicle& rb,
    SteeringType steering_type)
    : steering_type{ steering_type }
    , rb_{ rb }
    , turbine_power_{ NAN }
    , brake_amount_{ NAN }
    , throttle_relaxation_{ 0.f }
    , pitch_amount_{ NAN }
    , pitch_relaxation_{ 0.f }
    , yaw_amount_{ NAN }
    , yaw_relaxation_{ 0.f }
    , roll_amount_{ NAN }
    , roll_relaxation_{ 0.f }
{}

RigidBodyPlaneController::~RigidBodyPlaneController()
{}

void RigidBodyPlaneController::brake(float amount, float relaxation) {
    if (relaxation >= throttle_relaxation_) {
        turbine_power_ = 0.f;
        brake_amount_ = amount;
        throttle_relaxation_ = relaxation;
    }
}

void RigidBodyPlaneController::accelerate(float turbine_power, float relaxation) {
    if (relaxation >= throttle_relaxation_) {
        turbine_power_ = turbine_power;
        brake_amount_ = 0.f;
        throttle_relaxation_ = relaxation;
    }
}

void RigidBodyPlaneController::pitch(float amount, float relaxation) {
    if (relaxation >= pitch_relaxation_) {
        pitch_amount_ = amount;
        pitch_relaxation_ = relaxation;
    }
}

void RigidBodyPlaneController::yaw(float amount, float relaxation) {
    if (relaxation >= yaw_relaxation_) {
        yaw_amount_ = amount;
        yaw_relaxation_ = relaxation;
    }
}

void RigidBodyPlaneController::roll(float amount, float relaxation) {
    if (relaxation >= roll_relaxation_) {
        roll_amount_ = amount;
        roll_relaxation_ = relaxation;
    }
}

void RigidBodyPlaneController::reset_parameters(
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

void RigidBodyPlaneController::reset_relaxation(
    float throttle_relaxation,
    float pitch_relaxation,
    float yaw_relaxation,
    float roll_relaxation)
{
    throttle_relaxation_ = 0.f;
    pitch_relaxation_ = 0.f;
    yaw_relaxation_ = 0.f;
    roll_relaxation_ = 0.f;
}
