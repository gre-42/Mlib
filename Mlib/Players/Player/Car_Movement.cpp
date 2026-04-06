#include "Car_Movement.hpp"
#include <Mlib/Math/Sigmoid/Signed_Min.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Car_Controllers/Rigid_Body_Vehicle_Controller.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Threads/Throwing_Lock_Guard.hpp>
#include <stdexcept>

using namespace Mlib;

CarMovement::CarMovement(const DanglingBaseClassRef<Player>& player)
    : player_{player}
{}

CarMovement::~CarMovement() = default;

void CarMovement::step_on_brakes() {
    ThrowingLockGuard delete_lock{player_->delete_node_mutex_};
    if (!player_->has_scene_vehicle()) {
        throw std::runtime_error("step_on_brakes despite nullptr");
    }
    player_->rigid_body()->vehicle_controller(CURRENT_SOURCE_LOCATION)->step_on_brakes(1.f);
}

void CarMovement::drive_forward() {
    ThrowingLockGuard delete_lock{player_->delete_node_mutex_};
    if (!player_->has_scene_vehicle()) {
        throw std::runtime_error("drive_forward despite nullptr");
    }
    player_->rigid_body()->vehicle_controller(CURRENT_SOURCE_LOCATION)->drive(player_->vehicle_movement.surface_power_forward(), 1.f);
}

void CarMovement::drive_backwards() {
    ThrowingLockGuard delete_lock{player_->delete_node_mutex_};
    if (!player_->has_scene_vehicle()) {
        throw std::runtime_error("drive_backwards despite nullptr");
    }
    player_->rigid_body()->vehicle_controller(CURRENT_SOURCE_LOCATION)->drive(player_->vehicle_movement.surface_power_backward(), 1.f);
}

void CarMovement::roll_tires() {
    ThrowingLockGuard delete_lock{player_->delete_node_mutex_};
    if (!player_->has_scene_vehicle()) {
        throw std::runtime_error("roll despite nullptr");
    }
    player_->rigid_body()->vehicle_controller(CURRENT_SOURCE_LOCATION)->roll_tires();
}

void CarMovement::steer(float angle) {
    ThrowingLockGuard delete_lock{player_->delete_node_mutex_};
    if (max_tire_angle_.has_value()) {
        angle = signed_min(angle, *max_tire_angle_);
    } else if (!std::isfinite(angle)) {
        throw std::runtime_error("Infinite steering angle without max tire angle");
    }
    if (tire_angle_pid_.has_value()) {
        angle = (*tire_angle_pid_)(angle);
    }
    player_->rigid_body()->vehicle_controller(CURRENT_SOURCE_LOCATION)->steer(angle, 1.f);
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
    max_tire_angle_.reset();
    tire_angle_pid_.reset();
}

void CarMovement::set_max_tire_angle(float max_tire_angle)
{
    if (max_tire_angle_.has_value()) {
        throw std::runtime_error("max_tire_angle already set");
    }
    max_tire_angle_ = max_tire_angle;
}

void CarMovement::set_tire_angle_pid(const PidController<float, float>& tire_angle_pid) {
    if (tire_angle_pid_.has_value()) {
        throw std::runtime_error("tire_angle_pid already set");
    }
    tire_angle_pid_ = tire_angle_pid;
}
