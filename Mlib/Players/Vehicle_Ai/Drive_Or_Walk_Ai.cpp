#include "Drive_Or_Walk_Ai.hpp"
#include <Mlib/Geometry/Intersection/Intersect_Lines.hpp>
#include <Mlib/Memory/Destruction_Functions_Removeal_Tokens_Object.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Ai/Ai_Waypoint.hpp>
#include <Mlib/Physics/Ai/Control_Source.hpp>
#include <Mlib/Physics/Ai/Skill_Factor.hpp>
#include <Mlib/Physics/Rigid_Body/Actor_Task.hpp>
#include <Mlib/Physics/Rigid_Body/Actor_Type.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Vehicle_Domain.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Avatar_Controllers/Rigid_Body_Avatar_Controller.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Car_Controllers/Rigid_Body_Vehicle_Controller.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Scene_Graph/Driving_Direction.hpp>
#include <Mlib/Scene_Graph/Way_Point_Location.hpp>

using namespace Mlib;

DriveOrWalkAi::DriveOrWalkAi(
    const DanglingBaseClassRef<Player>& player,
    ScenePos waypoint_reached_radius,
    float rest_radius,
    float lookahead_velocity,
    float takeoff_velocity,
    float takeoff_velocity_delta,
    float max_velocity,
    float max_delta_velocity_brake,
    ScenePos collision_avoidance_radius_brake,
    ScenePos collision_avoidance_radius_wait,
    ScenePos collision_avoidance_radius_correct,
    float collision_avoidance_intersect_cos,
    float collision_avoidance_step_aside_cos,
    float collision_avoidance_step_aside_distance)
    : on_player_delete_vehicle_internals_{ player->delete_vehicle_internals, CURRENT_SOURCE_LOCATION }
    , player_{ player }
    , waypoint_reached_radius_{ waypoint_reached_radius }
    , rest_radius_{ rest_radius }
    , lookahead_velocity_{ lookahead_velocity }
    , takeoff_velocity_{ takeoff_velocity }
    , takeoff_velocity_delta_{ takeoff_velocity_delta }
    , max_velocity_{ max_velocity }
    , max_delta_velocity_brake_{ max_delta_velocity_brake }
    , collision_avoidance_radius_brake_{ collision_avoidance_radius_brake }
    , collision_avoidance_radius_wait_{ collision_avoidance_radius_wait }
    , collision_avoidance_radius_correct_{ collision_avoidance_radius_correct }
    , collision_avoidance_intersect_cos_{ collision_avoidance_intersect_cos }
    , collision_avoidance_step_aside_cos_{ collision_avoidance_step_aside_cos }
    , collision_avoidance_step_aside_distance_{ collision_avoidance_step_aside_distance }
{
    on_player_delete_vehicle_internals_.add([this]() { global_object_pool.remove(this); }, CURRENT_SOURCE_LOCATION);
}

DriveOrWalkAi::~DriveOrWalkAi() {
    on_destroy.clear();
}

static ActorTask get_initial_actor_task(const RigidBodyVehicle& rb) {
    switch (rb.current_vehicle_domain_) {
    case VehicleDomain::GROUND:
        return ActorTask::GROUND_CRUISE;
    case VehicleDomain::AIR:
        return ActorTask::AIR_CRUISE;
    case VehicleDomain::UNDEFINED:
        return ActorTask::UNDEFINED;
    case VehicleDomain::END:
        ; // Do nothing
    }
    THROW_OR_ABORT("Unknown vehicle domain: " + vehicle_domain_to_string(rb.current_vehicle_domain_));
}

VehicleAiMoveToStatus DriveOrWalkAi::move_to(
    const AiWaypoint& ai_waypoint,
    const SkillMap* skills,
    const StaticWorld& world)
{
    // if (waypoint_defined()) {
    //     g_beacons.push_back(Beacon::create(waypoint_.value(), "flag"));
    // }
    if (!player_->has_scene_vehicle()) {
        return VehicleAiMoveToStatus::SCENE_VEHICLE_IS_NULL;
    }
    if ((skills != nullptr) && !skills->skills(ControlSource::AI).can_drive) {
        return VehicleAiMoveToStatus::SKILL_IS_MISSING;
    }
    auto& player_rb = player_->rigid_body();
    if (player_rb.actor_task_ == ActorTask::UNDEFINED) {
        auto actor_task = get_initial_actor_task(player_rb);
        if (player_rb.has_autopilot(actor_task)) {
            player_rb.actor_task_ = actor_task;
        }
        // Return even if the "actor_task_" is not undefined, because
        // the "DriveOrWalkAi" might not have a "SkillFactor" for the new "actor_task_".
        return VehicleAiMoveToStatus::SKILL_IS_MISSING;
    }
    // Disabled, using "steer" instead to enable the PID-controller.
    // player_rb.vehicle_controller().reset_parameters(
    //     0.f, // surface_power
    //     0.f  // steer_angle
    // );
    player_rb.vehicle_controller().reset_relaxation(0.f, 0.f);
    if (!ai_waypoint.has_position_of_destination()) {
        player_->car_movement.step_on_brakes();
        player_->car_movement.steer(0.f);
        player_rb.vehicle_controller().apply();
        return VehicleAiMoveToStatus::WAYPOINT_IS_NAN;
    }
    auto waypoint_flags = ai_waypoint.flags();
    auto pod = ai_waypoint.position_of_destination(player_rb.waypoint_ofs_);
    VehicleAiMoveToStatus result = VehicleAiMoveToStatus::NONE;
    FixedArray<ScenePos, 3> pos3 = player_rb.rbp_.abs_position();
    ScenePos distance_to_waypoint2 = sum(squared(pos3 - funpack(pod)));
    SceneDir lookahead_fac2 = std::max(
        1.f,
        sum(squared(player_rb.rbp_.v_)) /
        squared(lookahead_velocity_));
    if (distance_to_waypoint2 < squared(waypoint_reached_radius_) * lookahead_fac2) {
        result |= VehicleAiMoveToStatus::WAYPOINT_REACHED;
    }
    if (std::isnan(player_->vehicle_movement.surface_power_forward()) ||
        std::isnan(player_->vehicle_movement.surface_power_backward()))
    {
        player_->car_movement.step_on_brakes();
        result |= VehicleAiMoveToStatus::POWER_IS_NAN;
    }
    // Stop when distance to waypoint is small enough (brake).
    if (!player_->ramming()) {
        if (distance_to_waypoint2 < squared(rest_radius_) * lookahead_fac2) {
            player_->car_movement.step_on_brakes();
            result |= VehicleAiMoveToStatus::RESTING_POSITION_REACHED;
        }
    }
    auto z3 = player_rb.rbp_.abs_z();
    auto z = FixedArray<float, 2>{ z3(0), z3(2) };
    auto p3 = player_rb.rbp_.abs_position();
    float d_wpt = 0;
    // Avoid collisions with other players (brake).
    for (const auto& [_, p] : player_->players().players()) {
        if (p.get() == &player_.get()) {
            continue;
        }
        if (!p->has_scene_vehicle()) {
            continue;
        }
        auto& p_rb = p->rigid_body();
        if (player_->ramming() &&
            (&p_rb == player_->target_rb()))
        {
            continue;
        }
        auto p_p3 = p_rb.rbp_.abs_position();
        auto d = p_p3 - p3;
        ScenePos dl2 = sum(squared(d));
        [&]() {
            if (dl2 < squared(collision_avoidance_radius_brake_)) {
                if (dot0d(d, z3.casted<ScenePos>()) < 0) {
                    player_->car_movement.step_on_brakes();
                    result |= VehicleAiMoveToStatus::STOPPED_TO_AVOID_COLLISION;
                    return;
                }
            }
            if (dl2 < squared(collision_avoidance_radius_wait_)) {
                auto p_z3 = p_rb.rbp_.abs_z();
                if (std::abs(dot0d(z3, p_z3)) < collision_avoidance_intersect_cos_) {
                    FixedArray<ScenePos, 2> intersection = uninitialized;
                    if (!intersect_rays(
                        intersection,
                        FixedArray<ScenePos, 2>{ p3(0), p3(2) },
                        FixedArray<ScenePos, 2>{ z3(0), z3(2) },
                        FixedArray<ScenePos, 2>{ p_p3(0), p_p3(2) },
                        FixedArray<ScenePos, 2>{ p_z3(0), p_z3(2) },
                        ScenePos(0),
                        ScenePos(0)))
                    {
                        player_->car_movement.step_on_brakes();
                        result |= VehicleAiMoveToStatus::STOPPED_TO_AVOID_COLLISION;
                        return;
                    }
                    auto iv0 = (intersection - FixedArray<ScenePos, 2>{ p3(0), p3(2) }).casted<float>();
                    auto iv1 = (intersection - FixedArray<ScenePos, 2>{ p_p3(0), p_p3(2) }).casted<float>();
                    if (dot0d(z, iv0) < 0) {
                        auto d2_0 = sum(squared(iv0));
                        auto d2_1 = sum(squared(iv1));
                        if (d2_0 > d2_1) {
                            player_->car_movement.step_on_brakes();
                            result |= VehicleAiMoveToStatus::STOPPED_TO_AVOID_COLLISION;
                            return;
                        }
                    }
                }
            }
            if (any(ai_waypoint.flags() & WayPointLocation::EXPLICIT_GROUND) &&
                (dl2 > 1e-12) &&
                (dl2 < squared(collision_avoidance_radius_correct_)))
            {
                if ((player_rb.avatar_controller_ != nullptr) ||
                    (dot0d(d, z3.casted<ScenePos>()) / std::sqrt(dl2) < -collision_avoidance_step_aside_cos_))
                {
                    if (player_->driving_direction() == DrivingDirection::CENTER || player_->driving_direction() == DrivingDirection::RIGHT) {
                        d_wpt = collision_avoidance_step_aside_distance_;
                    } else if (player_->driving_direction() == DrivingDirection::LEFT) {
                        d_wpt = -collision_avoidance_step_aside_distance_;
                    } else {
                        THROW_OR_ABORT("Unknown driving direction");
                    }
                    return;
                }
            }
        }();
    }
    if (player_rb.avatar_controller_ != nullptr) {
        player_rb.avatar_controller_->reset();
    }
    SceneDir zl2 = sum(squared(z));
    if (zl2 > 1e-12) {
        z /= std::sqrt(zl2);
        auto wpt = funpack(FixedArray<CompressedScenePos, 2>{ pod(0), pod(2) }) - FixedArray<ScenePos, 2>{ p3(0), p3(2) };
        auto m = FixedArray<ScenePos, 2, 2>::init(
            z(1), -z(0),
            z(0), z(1));
        wpt = dot1d(m, wpt);
        auto wpt2 = sum(squared(wpt));
        if (wpt2 > 1e-12) {
            wpt += FixedArray<ScenePos, 2>(-wpt(1), wpt(0)) / std::sqrt(wpt2) * ScenePos(d_wpt);
            auto wpt2c = std::sqrt(sum(squared(wpt)));
            if (wpt2c > 1e-12) {
                // Rotate waypoint back to global coordinates.
                auto wpt0_dir = (dot(wpt, m) / wpt2c).casted<float>();
                auto wpt_dir = (wpt / wpt2c).casted<float>();

                // Keep velocity within the specified range.
                if (!any(result & VehicleAiMoveToStatus::STOPPED_TO_AVOID_COLLISION)) {
                    bool is_accelerating_on_runway =
                        player_rb.has_autopilot(ActorTask::RUNWAY_TAKEOFF) &&
                        any(waypoint_flags & (WayPointLocation::RUNWAY | WayPointLocation::AIRWAY)) &&
                        !any(waypoint_flags & WayPointLocation::TAXIWAY) &&
                        any(ai_waypoint.latest_history_flags() & (WayPointLocation::RUNWAY | WayPointLocation::AIRWAY));
                    float target_vel;
                    if (ai_waypoint.has_velocity_at_destination()) {
                        target_vel = std::sqrt(sum(squared(ai_waypoint.velocity_at_destination())));
                    } else if (is_accelerating_on_runway) {
                        target_vel = takeoff_velocity_;
                    } else {
                        target_vel = max_velocity_;
                    }
                    float dvel;
                    if (player_rb.avatar_controller_ != nullptr) {
                        // Avatars steer through "strafing" with "increment_legs_z".
                        dvel = dot0d(FixedArray<SceneDir, 2>{player_rb.rbp_.v_(0), player_rb.rbp_.v_(2)}, wpt0_dir) - target_vel;
                    } else {
                        dvel = -dot0d(player_rb.rbp_.v_, player_rb.rbp_.abs_z()) - target_vel;
                    }
                    if (is_accelerating_on_runway && (std::abs(dvel) < takeoff_velocity_delta_)) {
                        player_rb.actor_task_ = ActorTask::RUNWAY_TAKEOFF;
                    }
                    if (dvel < 0) {
                        if (player_rb.avatar_controller_ != nullptr) {
                            player_rb.avatar_controller_->walk(player_->vehicle_movement.surface_power_forward(), 1.f);
                        } else {
                            player_->car_movement.drive_forward();
                        }
                    } else if (dvel < max_delta_velocity_brake_) {
                        if (player_rb.avatar_controller_ != nullptr) {
                            player_rb.avatar_controller_->walk(0.f, 1.f);
                        } else {
                            player_->car_movement.roll_tires();
                        }
                    } else {
                        if (player_rb.avatar_controller_ != nullptr) {
                            player_rb.avatar_controller_->stop();
                        } else {
                            player_->car_movement.step_on_brakes();
                        }
                    }
                }

                // Steer towards waypoint.
                // FixedArray<float, 3> wp{destination_position(0), 0, destination_position(1)};
                // auto m = vehicle_.rb->get_new_absolute_model_matrix();
                // auto v = inverted_scaled_se3(m);
                // auto wpt = dehomogenized_3(dot1d(v, homogenized_4(wp)));
                if (player_rb.avatar_controller_ != nullptr) {
                    player_rb.avatar_controller_->increment_legs_z(FixedArray<SceneDir, 3>{wpt_dir(0), SceneDir(0), wpt_dir(1)});
                    // player_rb.avatar_controller_->increment_legs_z(FixedArray<float, 3>{0.f, 0.f, -1.f});
                    if (player_->target_rb() == nullptr) {
                        player_rb.avatar_controller_->set_target_yaw(std::atan2(-wpt0_dir(0), -wpt0_dir(1)));
                    }
                    player_rb.avatar_controller_->apply();
                    return result;
                } else {
                    if (wpt(1) > 0) {
                        // The waypoint is behind us => full, inverted steering.
                        if (wpt(0) < 0) {
                            player_->car_movement.steer_left_full();
                            player_rb.vehicle_controller().apply();
                            return result;
                        } else {
                            player_->car_movement.steer_right_full();
                            player_rb.vehicle_controller().apply();
                            return result;
                        }
                    } else {
                        // The waypoint is in front of us => partial, inverted steering.
                        auto angle = (float)std::atan(std::abs(wpt(0) / wpt(1)));
                        if (wpt(0) < 0) {
                            player_->car_movement.steer_left_partial(angle);
                            player_rb.vehicle_controller().apply();
                            return result;
                        } else {
                            player_->car_movement.steer_right_partial(angle);
                            player_rb.vehicle_controller().apply();
                            return result;
                        }
                    }
                }
            }
        }
    }
    if (player_rb.avatar_controller_ != nullptr) {
        player_rb.avatar_controller_->apply();
    } else {
        player_->car_movement.steer(0.f);
        player_rb.vehicle_controller().apply();
    }
    return result;
}

std::vector<SkillFactor> DriveOrWalkAi::skills() const {
    return {
        SkillFactor{
            .scenario = SkillScenario{
                .actor_type = ActorType::TIRES,
                .actor_task = ActorTask::UNDEFINED},
            .factor = 1.f},
        SkillFactor{
            .scenario = SkillScenario{
                .actor_type = ActorType::TIRES,
                .actor_task = ActorTask::GROUND_CRUISE},
            .factor = 1.f},
        SkillFactor{
            .scenario = SkillScenario{
                .actor_type = ActorType::TIRES,
                .actor_task = ActorTask::RUNWAY_ACCELERATE},
            .factor = 1.f},
        SkillFactor{
            .scenario = SkillScenario{
                .actor_type = ActorType::TIRES,
                .actor_task = ActorTask::RUNWAY_TAKEOFF},
            .factor = 1.f}
    };
}
