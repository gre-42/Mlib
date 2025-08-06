#include "Flying_Missile_Ai.hpp"
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Scaled_Unit_Vector.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Ai/Ai_Waypoint.hpp>
#include <Mlib/Physics/Ai/Control_Source.hpp>
#include <Mlib/Physics/Ai/Skill_Factor.hpp>
#include <Mlib/Physics/Ai/Skill_Map.hpp>
#include <Mlib/Physics/Rigid_Body/Actor_Task.hpp>
#include <Mlib/Physics/Rigid_Body/Actor_Type.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Pulses.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Vehicle_Domain.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Missile_Controllers/Rigid_Body_Missile_Controller.hpp>
#include <Mlib/Scene_Graph/Instances/Static_World.hpp>
#include <Mlib/Scene_Graph/Way_Point_Location.hpp>

using namespace Mlib;

FlyingMissileAi::FlyingMissileAi(
    RigidBodyVehicle& rigid_body,
    Interp<float, float> dy,
    double eta_max,
    RigidBodyMissileController& controller,
    float waypoint_reached_radius,
    float resting_position_reached_radius,
    float maximum_velocity)
    : on_destroy_rigid_body_{ rigid_body.on_destroy, CURRENT_SOURCE_LOCATION }
    , dy_{ std::move(dy) }
    , eta_max_{ eta_max }
    , waypoint_reached_radius_squared_{ squared(waypoint_reached_radius) }
    , resting_position_reached_radius_squared_{ squared(resting_position_reached_radius) }
    , controller_{ controller }
    , rigid_body_{ rigid_body }
    , maximum_velocity_{ maximum_velocity }
{
    on_destroy_rigid_body_.add([this]() { global_object_pool.remove(this); }, CURRENT_SOURCE_LOCATION);
}

FlyingMissileAi::~FlyingMissileAi() {
    on_destroy.clear();
}

VehicleAiMoveToStatus FlyingMissileAi::move_to(
    const AiWaypoint& ai_waypoint,
    const SkillMap* skills,
    const StaticWorld& world,
    float dt)
{
    controller_.reset_parameters();
    controller_.reset_relaxation();

    if ((skills != nullptr) && !skills->skills(ControlSource::AI).can_drive) {
        return VehicleAiMoveToStatus::SKILL_IS_MISSING;
    }
    if (!ai_waypoint.has_position_of_destination()) {
        controller_.throttle_engine(0.f, 1.f);
        controller_.set_desired_direction(fixed_zeros<float, 3>(), 1.f);
        controller_.apply(dt);
        return VehicleAiMoveToStatus::WAYPOINT_IS_NAN;
    }
    auto pod = ai_waypoint.position_of_destination(rigid_body_.waypoint_ofs_);
    auto vod = ai_waypoint.velocity_of_destination(fixed_zeros<float, 3>());
    auto flags = ai_waypoint.flags();

    if (dot0d(rigid_body_.rbp_.v_com_, rigid_body_.rbp_.rotation_.column(2)) > -maximum_velocity_) {
        controller_.throttle_engine(INFINITY, 1.f);
    } else {
        controller_.throttle_engine(0.f, 1.f);
    }

    auto distance2 = sum(squared(funpack(pod) - rigid_body_.rbp_.abs_position()));
    auto eta = std::min(eta_max_, std::sqrt(distance2 / sum(squared(rigid_body_.rbp_.v_com_))));
    auto corrected_position_of_destination = funpack(pod) + eta * vod.casted<ScenePos>();

    auto dir_d = corrected_position_of_destination - rigid_body_.rbp_.abs_position();
    auto l2_d = sum(squared(dir_d));
    VehicleAiMoveToStatus result = VehicleAiMoveToStatus::NONE;
    if (l2_d < waypoint_reached_radius_squared_) {
        result |= VehicleAiMoveToStatus::WAYPOINT_REACHED;
    }
    if (l2_d < resting_position_reached_radius_squared_) {
        return result | VehicleAiMoveToStatus::RESTING_POSITION_REACHED;
    }
    auto dir = (dir_d / std::sqrt(l2_d)).casted<float>();

    auto vz = dot0d(rigid_body_.rbp_.v_com_, rigid_body_.rbp_.rotation_.column(2));
    if (vz <= 0.f) {
        if ((world.gravity == nullptr) || (world.gravity->magnitude == 0.f)) {
            THROW_OR_ABORT("Flying missile AI requires nonzero gravity");
        }
        dir -= world.gravity->direction * dy_(-vz);
        auto l = std::sqrt(sum(squared(dir)));
        if (l < 1e-12) {
            THROW_OR_ABORT("Direction length too small, please choose a smaller dy value");
        }
        dir /= l;
    }
    if (any(flags & WayPointLocation::AIRWAY) &&
        (rigid_body_.current_vehicle_domain_ == VehicleDomain::GROUND))
    {
        dir(1) += 0.5f;
        auto l = std::sqrt(sum(squared(dir)));
        if (l < 1e-12) {
            THROW_OR_ABORT("Direction length too small during takeoff");
        }
        dir /= l;
    }
    controller_.set_desired_direction(dir, 1.f);
    controller_.apply(dt);
    return result;
}


std::vector<SkillFactor> FlyingMissileAi::skills() const {
    return {
        SkillFactor{
            .scenario = SkillScenario{
                .actor_type = ActorType::WINGS,
                .actor_task = ActorTask::RUNWAY_TAKEOFF},
            .factor = 1.f},
        SkillFactor{
            .scenario = SkillScenario{
                .actor_type = ActorType::WINGS,
                .actor_task = ActorTask::AIR_CRUISE},
            .factor = 1.f}
    };
}
