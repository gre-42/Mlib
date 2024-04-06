#include "Missile_Controller.hpp"
#include <Mlib/Math/Signed_Min.hpp>
#include <Mlib/Physics/Actuators/Engine_Power_Intent.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Vehicle_Domain.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Steering_Type.hpp>
#include <Mlib/Scene_Graph/Animation/Animation_State_Updater.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

MissileController::MissileController(
    RigidBodyVehicle& rb,
    const PidController<float, FixedArray<float, 3>>& pid,
    const MissileWingController& left_front,
    const MissileWingController& right_front,
    const MissileWingController& down_front,
    const MissileWingController& up_front,
    const MissileWingController& left_rear,
    const MissileWingController& right_rear,
    const MissileWingController& down_rear,
    const MissileWingController& up_rear)
    : RigidBodyMissileController{ rb, pid }
    , left_front_{ left_front }
    , right_front_{ right_front }
    , down_front_{ down_front }
    , up_front_{ up_front }
    , left_rear_{ left_rear }
    , right_rear_{ right_rear }
    , down_rear_{ down_rear }
    , up_rear_{ up_rear }
{}

MissileController::~MissileController() = default;

void MissileController::apply() {
    auto rel_dir = dot(desired_direction_, rb_.rbp_.rotation_);
    FixedArray<float, 2> fake_dir{ rel_dir(0), rel_dir(1) };
    if (rel_dir(2) < 1e-12) {
        float l2 = std::sqrt(sum(squared(fake_dir)));
        if (l2 < 1e-12) {
            fake_dir = { 1.f, 0.f };
        } else {
            fake_dir /= l2;
        }
    }
    auto set_angle_of_attack = [this, &fake_dir](MissileWingController& mwc, size_t axis) {
        rb_.set_wing_angle_of_attack(mwc.i, mwc.gain * fake_dir(axis));
        };
    set_angle_of_attack(left_front_, 1);
    set_angle_of_attack(right_front_, 1);
    set_angle_of_attack(down_front_, 0);
    set_angle_of_attack(up_front_, 0);
    set_angle_of_attack(left_rear_, 1);
    set_angle_of_attack(right_rear_, 1);
    set_angle_of_attack(down_rear_, 0);
    set_angle_of_attack(up_rear_, 0);
}
