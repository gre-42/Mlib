#include "Drive_Or_Walk_Ai.hpp"
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Avatar_Controllers/Rigid_Body_Avatar_Controller.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Car_Controllers/Rigid_Body_Vehicle_Controller.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Players/Scene_Vehicle/Control_Source.hpp>
#include <Mlib/Scene_Graph/Driving_Direction.hpp>

using namespace Mlib;

DriveOrWalkAi::DriveOrWalkAi(Player& player)
	: player_{ player }
{}

DriveOrWalkAi::~DriveOrWalkAi() = default;

VehicleAiMoveToStatus DriveOrWalkAi::move_to(
	const FixedArray<double, 3>& destination_position,
    const FixedArray<float, 3>& destination_velocity)
{
    // if (!any(Mlib::isnan(waypoint_))) {
    //     g_beacons.push_back(Beacon::create(waypoint_, "flag"));
    // }
    if (!player_.has_scene_vehicle()) {
        return VehicleAiMoveToStatus::SCENE_VEHICLE_IS_NULL;
    }
    if (!player_.skills(ControlSource::AI).can_drive) {
        return VehicleAiMoveToStatus::SKILL_MISSING;
    }
    auto& player_rb = player_.rigid_body();
    // Disabled, using "steer" instead to enable the PID-controller.
    // player_rb.vehicle_controller().reset_parameters(
    //     0.f, // surface_power
    //     0.f  // steer_angle
    // );
    auto step_on_brakes_and_apply = [this, &player_rb](){
        player_.car_movement.step_on_brakes();
        player_.car_movement.steer(0.f);
        player_rb.vehicle_controller().apply();
        };
    player_rb.vehicle_controller().reset_relaxation(0.f, 0.f);
    if (any(Mlib::isnan(destination_position))) {
        step_on_brakes_and_apply();
        return VehicleAiMoveToStatus::WAYPOINT_IS_NAN;
    }
    VehicleAiMoveToStatus result = VehicleAiMoveToStatus::NONE;
    FixedArray<double, 3> pos3 = player_rb.rbp_.abs_position();
    double distance_to_waypoint2 = sum(squared(pos3 - destination_position));
    float lookahead_fac2 = std::max(
        1.f,
        sum(squared(player_rb.rbp_.v_)) /
        squared(player_.driving_mode().lookahead_velocity));
    if (distance_to_waypoint2 < squared(player_.driving_mode().waypoint_reached_radius) * lookahead_fac2) {
        result |= VehicleAiMoveToStatus::DESTINATION_REACHED;
    }
    if (std::isnan(player_.vehicle_movement.surface_power_forward()) ||
        std::isnan(player_.vehicle_movement.surface_power_backward()))
    {
        step_on_brakes_and_apply();
        return result | VehicleAiMoveToStatus::POWER_IS_NAN;
    }
    // Stop when distance to waypoint is small enough (brake).
    if (!player_.ramming()) {
        if (distance_to_waypoint2 < squared(player_.driving_mode().rest_radius) * lookahead_fac2) {
            step_on_brakes_and_apply();
            return result | VehicleAiMoveToStatus::RESTING_POSITION_REACHED;
        }
    }
    float d_wpt = 0;
    // Avoid collisions with other players (brake).
    for (const auto& [_, p] : player_.players().players()) {
        if (p.get() == &player_) {
            continue;
        }
        if (!p->has_scene_vehicle()) {
            continue;
        }
        auto& p_rb = p->rigid_body();
        if (player_.ramming() &&
            (&p_rb == player_.target_rb()))
        {
            continue;
        }
        FixedArray<double, 3> d = p_rb.rbp_.abs_position() - player_rb.rbp_.abs_position();
        double dl2 = sum(squared(d));
        if (dl2 < squared(player_.driving_mode().collision_avoidance_radius_brake)) {
            auto z = player_rb.rbp_.abs_z();
            if (dot0d(d, z.casted<double>()) < 0) {
                step_on_brakes_and_apply();
                return VehicleAiMoveToStatus::STOPPED_TO_AVOID_COLLISION;
            }
        } else if (dl2 < squared(player_.driving_mode().collision_avoidance_radius_correct)) {
            if (dl2 > 1e-12) {
                if ((player_rb.avatar_controller_ != nullptr) ||
                    (dot0d(d, player_rb.rbp_.abs_z().casted<double>()) / std::sqrt(dl2) < -player_.driving_mode().collision_avoidance_cos))
                {
                    if (player_.driving_direction() == DrivingDirection::CENTER || player_.driving_direction() == DrivingDirection::RIGHT) {
                        d_wpt = player_.driving_mode().collision_avoidance_delta;
                    } else if (player_.driving_direction() == DrivingDirection::LEFT) {
                        d_wpt = -player_.driving_mode().collision_avoidance_delta;
                    } else {
                        THROW_OR_ABORT("Unknown driving direction");
                    }
                }
            }
        }
    }
    if (player_rb.avatar_controller_ != nullptr) {
        player_rb.avatar_controller_->reset();
    }
    // Keep velocity within the specified range.
    {
        float target_vel = all(isnan(destination_velocity))
            ? player_.driving_mode().max_velocity
            : std::sqrt(sum(squared(destination_velocity)));
        float dvel = -dot0d(player_rb.rbp_.v_, player_rb.rbp_.abs_z()) - target_vel;
        if (dvel < 0) {
            if (player_rb.avatar_controller_ != nullptr) {
                player_rb.avatar_controller_->walk(player_.vehicle_movement.surface_power_forward(), 1.f);
            } else {
                player_.car_movement.drive_forward();
            }
        } else if (dvel < player_.driving_mode().max_delta_velocity_brake) {
            if (player_rb.avatar_controller_ != nullptr) {
                player_rb.avatar_controller_->walk(0.f, 1.f);
            } else {
                player_.car_movement.roll_tires();
            }
        } else {
            if (player_rb.avatar_controller_ != nullptr) {
                player_rb.avatar_controller_->stop();
            } else {
                player_.car_movement.step_on_brakes();
            }
        }
    }
    // Steer towards waypoint.
    // FixedArray<float, 3> wp{destination_position(0), 0, destination_position(1)};
    // auto m = vehicle_.rb->get_new_absolute_model_matrix();
    // auto v = inverted_scaled_se3(m);
    // auto wpt = dehomogenized_3(dot1d(v, homogenized_4(wp)));
    auto z3 = player_rb.rbp_.abs_z();
    FixedArray<float, 2> z{z3(0), z3(2)};
    float zl2 = sum(squared(z));
    if (zl2 > 1e-12) {
        z /= std::sqrt(zl2);
        auto p = player_rb.rbp_.abs_position();
        auto wpt = FixedArray<double, 2>{destination_position(0), destination_position(2)} - FixedArray<double, 2>{p(0), p(2)};
        auto m = FixedArray<double, 2, 2>::init(
            z(1), -z(0),
            z(0), z(1));
        wpt = dot1d(m, wpt);
        wpt += FixedArray<double, 2>(-wpt(1), wpt(0)) * double(d_wpt);
        double wpt2 = sum(squared(wpt));
        if (wpt2 > 1e-12) {
            if (player_rb.avatar_controller_ != nullptr) {
                player_rb.avatar_controller_->increment_legs_z((FixedArray<double, 3>{wpt(0), 0., wpt(1)} / std::sqrt(wpt2)).casted<float>());
                // player_rb.avatar_controller_->increment_legs_z(FixedArray<float, 3>{0.f, 0.f, -1.f});
                if (player_.target_rb() == nullptr) {
                    // Rotate waypoint back to global coordinates.
                    auto wpt0 = dot(wpt, m);
                    player_rb.avatar_controller_->set_target_yaw((float)std::atan2(-wpt0(0), -wpt0(1)));
                }
                player_rb.avatar_controller_->apply();
                return result;
            } else {
                if (wpt(1) > 0) {
                    // The waypoint is behind us => full, inverted steering.
                    if (wpt(0) < 0) {
                        player_.car_movement.steer_left_full();
                        player_rb.vehicle_controller().apply();
                        return result;
                    } else {
                        player_.car_movement.steer_right_full();
                        player_rb.vehicle_controller().apply();
                        return result;
                    }
                } else {
                    // The waypoint is in front of us => partial, inverted steering.
                    float angle = (float)std::atan(std::abs(wpt(0) / wpt(1)));
                    if (wpt(0) < 0) {
                        player_.car_movement.steer_left_partial(angle);
                        player_rb.vehicle_controller().apply();
                        return result;
                    } else {
                        player_.car_movement.steer_right_partial(angle);
                        player_rb.vehicle_controller().apply();
                        return result;
                    }
                }
            }
        }
    }
    if (player_rb.avatar_controller_ != nullptr) {
        player_rb.avatar_controller_->apply();
    } else {
        player_.car_movement.steer(0.f);
        player_rb.vehicle_controller().apply();
    }
    return result;
}
