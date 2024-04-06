#include "Rigid_Body_Missile_Controller.hpp"
#include <Mlib/Math/Fixed_Math.hpp>

using namespace Mlib;

RigidBodyMissileController::RigidBodyMissileController(RigidBodyVehicle& rb)
    : rb_{ rb }
    , rocket_engine_power_{ NAN }
    , rocket_engine_power_relaxation_{ 0.f }
    , desired_direction_{ fixed_nans<float, 3>() }
    , desired_direction_relaxation_{ 0.f }
{}

RigidBodyMissileController::~RigidBodyMissileController() = default;

void RigidBodyMissileController::set_desired_direction(
    const FixedArray<float, 3>& dir,
    float relaxation)
{
    if (relaxation >= desired_direction_relaxation_) {
        desired_direction_ = dir;
        desired_direction_relaxation_ = relaxation;
    }
}

void RigidBodyMissileController::reset_parameters() {
    rocket_engine_power_ = NAN;
    desired_direction_ = NAN;
}

void RigidBodyMissileController::reset_relaxation() {
    rocket_engine_power_relaxation_ = 0.f;
    desired_direction_relaxation_ = 0.f;
}

void RigidBodyMissileController::throttle_engine(
    float rocket_engine_power,
    float relaxation)
{
    if (relaxation > rocket_engine_power_relaxation_) {
        rocket_engine_power_ = rocket_engine_power;
        rocket_engine_power_relaxation_ = relaxation;
    }
}
