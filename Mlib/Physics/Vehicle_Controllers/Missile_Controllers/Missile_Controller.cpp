#include "Missile_Controller.hpp"
#include <Mlib/Physics/Actuators/Engine_Power_Intent.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

MissileController::MissileController(
    RigidBodyVehicle& rb,
    std::vector<MissileWingController> wing_controllers,
    VariableAndHash<std::string> engine_name)
    : RigidBodyMissileController{ rb }
    , wing_controllers_{ std::move(wing_controllers) }
    , engine_name_{ std::move(engine_name) }
{}

MissileController::~MissileController() = default;

void MissileController::apply() {
    rb_.set_surface_power(engine_name_, EnginePowerIntent{
        .surface_power = rocket_engine_power_,
        .drive_relaxation = rocket_engine_power_relaxation_ });

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
    auto roll_strength = rb_.rbp_.rotation_(1, 0);
    for (const auto& wing_controller : wing_controllers_) {
        rb_.set_wing_angle_of_attack(
            wing_controller.i,
            dot0d(wing_controller.gain, fake_dir) +
            wing_controller.antiroll_angle * roll_strength);
    }
}
