#include "Rigid_Body_Avatar_Controller.hpp"
#include <cmath>

using namespace Mlib;

RigidBodyAvatarController::RigidBodyAvatarController()
: legs_z_(NAN),
  target_yaw_{NAN},
  target_pitch_{NAN},
  dyaw_{NAN},
  dpitch_{NAN},
  surface_power_{NAN}
{}

RigidBodyAvatarController::~RigidBodyAvatarController()
{}

void RigidBodyAvatarController::increment_legs_z(const FixedArray<float, 3>& dz) {
    legs_z_ += dz;
}

void RigidBodyAvatarController::walk(float surface_power) {
    surface_power_ = surface_power;
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

void RigidBodyAvatarController::increment_yaw(float dyaw) {
    dyaw_ = dyaw;
}

void RigidBodyAvatarController::increment_pitch(float dpitch) {
    dpitch_ = dpitch;
}

void RigidBodyAvatarController::reset() {
    legs_z_ = 0.f;
    surface_power_ = 0;
    target_yaw_ = NAN;
    target_pitch_ = NAN;
    dyaw_ = NAN;
    dpitch_ = NAN;
}
