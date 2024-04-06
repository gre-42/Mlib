#include "Missile_Controller.hpp"
#include <Mlib/Physics/Actuators/Engine_Power_Intent.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

MissileController::MissileController(
    RigidBodyVehicle& rb,
    const MissileWingController& left_front,
    const MissileWingController& right_front,
    const MissileWingController& down_front,
    const MissileWingController& up_front,
    const MissileWingController& left_rear,
    const MissileWingController& right_rear,
    const MissileWingController& down_rear,
    const MissileWingController& up_rear)
    : RigidBodyMissileController{ rb }
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
    rb_.set_surface_power("rocket_engine", EnginePowerIntent{
        .surface_power = rocket_engine_power_,
        .drive_relaxation = rocket_engine_power_relaxation_});

    auto rel_dir = dot(desired_direction_, rb_.rbp_.rotation_);
    FixedArray<float, 2> fake_dir{ rel_dir(0), rel_dir(1) };
    // Check if the target is behind the missile
    if (rel_dir(2) > -1e-12) {
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
