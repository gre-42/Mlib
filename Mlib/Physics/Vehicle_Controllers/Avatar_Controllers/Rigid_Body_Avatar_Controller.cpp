#include "Rigid_Body_Avatar_Controller.hpp"
#include <Mlib/Math/Fixed_Math.hpp>

using namespace Mlib;

RigidBodyAvatarController::RigidBodyAvatarController()
    : legs_z_(0.f)
    , target_yaw_{ NAN }
    , target_pitch_{ NAN }
    , dyaw_{ NAN }
    , dyaw_relaxation_{ 0.f }
    , dpitch_{ NAN }
    , dpitch_relaxation_{ 0.f }
    , surface_power_{ 0.f }
    , drive_relaxation_{ 0.f }
{}

RigidBodyAvatarController::~RigidBodyAvatarController() = default;

void RigidBodyAvatarController::increment_legs_z(const FixedArray<float, 3>& dz) {
    legs_z_ += dz;
}

void RigidBodyAvatarController::walk(float surface_power, float relaxation) {
    if (relaxation < drive_relaxation_) {
        return;
    }
    surface_power_ = surface_power;
    drive_relaxation_ = relaxation;
}

void RigidBodyAvatarController::stop() {
    surface_power_ = NAN;
}

void RigidBodyAvatarController::set_target_yaw(float target_yaw) {
    target_yaw_ = target_yaw;
}

void RigidBodyAvatarController::set_target_pitch(float target_pitch) {
    target_pitch_ = target_pitch;
}

void RigidBodyAvatarController::increment_yaw(float dyaw, float relaxation) {
    if (relaxation > dyaw_relaxation_) {
        dyaw_ = dyaw;
        dyaw_relaxation_ = relaxation;
    }
}

void RigidBodyAvatarController::increment_pitch(float dpitch, float relaxation) {
    if (relaxation > dpitch_relaxation_) {
        dpitch_ = dpitch;
        dpitch_relaxation_ = relaxation;
    }
}

void RigidBodyAvatarController::reset() {
    legs_z_ = 0.f;
    surface_power_ = 0.f;
    drive_relaxation_ = 0.f;
    target_yaw_ = NAN;
    target_pitch_ = NAN;
    dyaw_ = NAN;
    dyaw_relaxation_ = 0.f;
    dpitch_ = NAN;
    dpitch_relaxation_ = 0.f;
}
