#pragma once
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Physics/Ai/IVehicle_Ai.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>

namespace Mlib {

class Player;
template <class T>
class DanglingBaseClassRef;

class DriveOrWalkAi final: public IVehicleAi {
    DriveOrWalkAi(const DriveOrWalkAi&) = delete;
    DriveOrWalkAi& operator = (const DriveOrWalkAi&) = delete;
public:
    explicit DriveOrWalkAi(
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
        float collision_avoidance_step_aside_distance);
    virtual ~DriveOrWalkAi() override;
    virtual VehicleAiMoveToStatus move_to(
        const AiWaypoint& ai_waypoint,
        const SkillMap* skills,
        const StaticWorld& world,
        float dt) override;
    virtual std::vector<SkillFactor> skills() const override;
private:
    DestructionFunctionsRemovalTokens on_player_delete_vehicle_internals_;
    DanglingBaseClassRef<Player> player_;
    ScenePos waypoint_reached_radius_;
    float rest_radius_;
    float lookahead_velocity_;
    float takeoff_velocity_;
    float takeoff_velocity_delta_;
    float max_velocity_;
    float max_delta_velocity_brake_;
    ScenePos collision_avoidance_radius_brake_;
    ScenePos collision_avoidance_radius_wait_;
    ScenePos collision_avoidance_radius_correct_;
    float collision_avoidance_intersect_cos_;
    float collision_avoidance_step_aside_cos_;
    float collision_avoidance_step_aside_distance_;
};

}
