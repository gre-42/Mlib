#include "Player_Movement.hpp"
#include <Mlib/Math/Signed_Min.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Avatar_Controllers/Rigid_Body_Avatar_Controller.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Car_Controllers/Rigid_Body_Vehicle_Controller.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>

using namespace Mlib;

PlayerMovement::PlayerMovement(Player& player)
: player_{player},
  surface_power_forward_{ NAN },
  surface_power_backward_{ NAN },
  max_tire_angle_{ NAN },
  tire_angle_pid_{ NAN, NAN, NAN, NAN }
{}

PlayerMovement::~PlayerMovement()
{}

void PlayerMovement::run_move(
    float yaw,
    float pitch,
    float forwardmove,
    float sidemove)
{
    player_.delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!player_.has_rigid_body()) {
        throw std::runtime_error("run_move despite rigid body nullptr");
    }

    player_.rigid_body().avatar_controller().reset();

    player_.rigid_body().avatar_controller().set_target_yaw(yaw);
    player_.rigid_body().avatar_controller().set_target_pitch(pitch);

    FixedArray<float, 3> direction{ sidemove, 0.f, -forwardmove };
    float len2 = sum(squared(direction));
    if (len2 < 1e-12) {
        player_.rigid_body().avatar_controller().stop();
    } else {
        float len = std::sqrt(len2);
        player_.rigid_body().avatar_controller().increment_legs_z(direction / len);
        player_.rigid_body().avatar_controller().walk(surface_power_forward_);
    }
    player_.rigid_body().avatar_controller().apply();
}

void PlayerMovement::step_on_brakes() {
    player_.delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!player_.has_rigid_body()) {
        throw std::runtime_error("step_on_brakes despite nullptr");
    }
    player_.vehicle_.rb->vehicle_controller().step_on_brakes();
}

void PlayerMovement::drive_forward() {
    player_.delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!player_.has_rigid_body()) {
        throw std::runtime_error("drive_forward despite nullptr");
    }
    player_.vehicle_.rb->vehicle_controller().drive(surface_power_forward_);
}

void PlayerMovement::drive_backwards() {
    player_.delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!player_.has_rigid_body()) {
        throw std::runtime_error("drive_backwards despite nullptr");
    }
    player_.vehicle_.rb->vehicle_controller().drive(surface_power_backward_);
}

void PlayerMovement::roll_tires() {
    player_.delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!player_.has_rigid_body()) {
        throw std::runtime_error("roll despite nullptr");
    }
    player_.vehicle_.rb->vehicle_controller().roll_tires();
}

void PlayerMovement::steer(float angle) {
    player_.delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (std::isnan(max_tire_angle_)) {
        throw std::runtime_error("PlayerMovement::steer: max tire angle not set");
    }
    player_.vehicle_.rb->vehicle_controller().steer(tire_angle_pid_(signed_min(angle, max_tire_angle_)));
}

void PlayerMovement::steer_left_full() {
    steer(INFINITY);
}

void PlayerMovement::steer_right_full() {
    steer(-INFINITY);
}

void PlayerMovement::steer_left_partial(float angle) {
    steer(angle);
}

void PlayerMovement::steer_right_partial(float angle) {
    steer(-angle);
}

void PlayerMovement::reset_node() {
    surface_power_forward_ = NAN;
    surface_power_backward_ = NAN;
    max_tire_angle_ = NAN;
    tire_angle_pid_ = PidController<float, float>{NAN, NAN, NAN, NAN};
}

void PlayerMovement::set_vehicle_control_parameters(
    float surface_power_forward,
    float surface_power_backward,
    float max_tire_angle,
    const PidController<float, float>& tire_angle_pid)
{
    if (!std::isnan(surface_power_forward_)) {
        throw std::runtime_error("surface_power_forward already set");
    }
    surface_power_forward_ = surface_power_forward;
    if (!std::isnan(surface_power_backward_)) {
        throw std::runtime_error("surface_power_backward already set");
    }
    surface_power_backward_ = surface_power_backward;
    if (!std::isnan(max_tire_angle_)) {
        throw std::runtime_error("max_tire_angle_ already set");
    }
    max_tire_angle_ = max_tire_angle;
    tire_angle_pid_ = tire_angle_pid;
}
