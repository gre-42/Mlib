#include "Destination_Reached_Ai.hpp"
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Ai/Ai_Waypoint.hpp>
#include <Mlib/Physics/Ai/Skill_Factor.hpp>
#include <Mlib/Physics/Ai/Skill_Map.hpp>
#include <Mlib/Physics/Ai/Skills.hpp>
#include <Mlib/Physics/Rigid_Body/Actor_Task.hpp>
#include <Mlib/Physics/Rigid_Body/Actor_Type.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Pulses.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>

using namespace Mlib;

DestinationReachedAi::DestinationReachedAi(
    RigidBodyVehicle& rigid_body,
    ControlSource control_source,
    ScenePos destination_reached_radius)
    : on_destroy_rigid_body_{ rigid_body.on_destroy, CURRENT_SOURCE_LOCATION }
    , rigid_body_{ rigid_body }
    , control_source_{ control_source }
    , destination_reached_radius_squared_{ squared(destination_reached_radius) }
{
    on_destroy_rigid_body_.add([this]() { global_object_pool.remove(this); }, CURRENT_SOURCE_LOCATION);
}

DestinationReachedAi::~DestinationReachedAi() {
    on_destroy.clear();
}

VehicleAiMoveToStatus DestinationReachedAi::move_to(
    const AiWaypoint& ai_waypoint,
    const SkillMap* skills,
    const StaticWorld& world)
{
    if ((skills != nullptr) && !skills->skills(control_source_).can_drive) {
        return VehicleAiMoveToStatus::SKILL_IS_MISSING;
    }
    if (!ai_waypoint.has_position_of_destination()) {
        return VehicleAiMoveToStatus::WAYPOINT_IS_NAN;
    }
    auto pod = ai_waypoint.position_of_destination(rigid_body_.waypoint_ofs_);

    auto distance2 = sum(squared(funpack(pod) - rigid_body_.rbp_.abs_position()));

    if (distance2 < destination_reached_radius_squared_) {
        return VehicleAiMoveToStatus::WAYPOINT_REACHED;
    }
    return VehicleAiMoveToStatus::NONE;
}

std::vector<SkillFactor> DestinationReachedAi::skills() const {
    return {
        SkillFactor{
            .scenario = SkillScenario{
                .actor_type = ActorType::DESTINATION_REACHED_STATUS,
                .actor_task = ActorTask::AIR_CRUISE},
            .factor = 1.f},
        SkillFactor{
            .scenario = SkillScenario{
                .actor_type = ActorType::DESTINATION_REACHED_STATUS,
                .actor_task = ActorTask::GROUND_CRUISE},
            .factor = 1.f},
        SkillFactor{
            .scenario = SkillScenario{
                .actor_type = ActorType::DESTINATION_REACHED_STATUS,
                .actor_task = ActorTask::RUNWAY_ACCELERATE},
            .factor = 1.f},
        SkillFactor{
            .scenario = SkillScenario{
                .actor_type = ActorType::DESTINATION_REACHED_STATUS,
                .actor_task = ActorTask::RUNWAY_TAKEOFF},
            .factor = 1.f},
    };
}
