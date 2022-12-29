#include "Car_Movement.hpp"
#include <Mlib/Math/Signed_Min.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Car_Controllers/Rigid_Body_Vehicle_Controller.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

CarMovement::CarMovement(Player& player)
: player_{player},
  max_tire_angle_{ NAN },
  tire_angle_pid_{ NAN, NAN, NAN, NAN }
{}

CarMovement::~CarMovement()
{}

void CarMovement::step_on_brakes() {
    player_.delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!player_.has_rigid_body()) {
        THROW_OR_ABORT("step_on_brakes despite nullptr");
    }
    player_.vehicle_.rb->vehicle_controller().step_on_brakes();
}

void CarMovement::drive_forward() {
    player_.delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!player_.has_rigid_body()) {
        THROW_OR_ABORT("drive_forward despite nullptr");
    }
    player_.vehicle_.rb->vehicle_controller().drive(player_.vehicle_movement.surface_power_forward(), 1.f);
}

void CarMovement::drive_backwards() {
    player_.delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!player_.has_rigid_body()) {
        THROW_OR_ABORT("drive_backwards despite nullptr");
    }
    player_.vehicle_.rb->vehicle_controller().drive(player_.vehicle_movement.surface_power_backward(), 1.f);
}

void CarMovement::roll_tires() {
    player_.delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!player_.has_rigid_body()) {
        THROW_OR_ABORT("roll despite nullptr");
    }
    player_.vehicle_.rb->vehicle_controller().roll_tires();
}

void CarMovement::steer(float angle) {
    player_.delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (std::isnan(max_tire_angle_)) {
        THROW_OR_ABORT("CarMovement::steer: max tire angle not set");
    }
    player_.vehicle_.rb->vehicle_controller().steer(tire_angle_pid_(signed_min(angle, max_tire_angle_)), 1.f);
}

void CarMovement::steer_left_full() {
    steer(INFINITY);
}

void CarMovement::steer_right_full() {
    steer(-INFINITY);
}

void CarMovement::steer_left_partial(float angle) {
    steer(angle);
}

void CarMovement::steer_right_partial(float angle) {
    steer(-angle);
}

void CarMovement::reset_node() {
    max_tire_angle_ = NAN;
    tire_angle_pid_ = PidController<float, float>{NAN, NAN, NAN, NAN};
}

void CarMovement::set_control_parameters(
    float max_tire_angle,
    const PidController<float, float>& tire_angle_pid)
{
    if (!std::isnan(max_tire_angle_)) {
        THROW_OR_ABORT("max_tire_angle_ already set");
    }
    max_tire_angle_ = max_tire_angle;
    tire_angle_pid_ = tire_angle_pid;
}
