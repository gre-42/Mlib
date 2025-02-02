#pragma once
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Physics/Ai/IVehicle_Ai.hpp>
#include <Mlib/Scene_Precision.hpp>

namespace Mlib {

class RigidBodyVehicle;
enum class ControlSource;

class DestinationReachedAi final: public IVehicleAi {
    DestinationReachedAi(const DestinationReachedAi&) = delete;
    DestinationReachedAi& operator = (const DestinationReachedAi&) = delete;
public:
    explicit DestinationReachedAi(
        RigidBodyVehicle& rigid_body,
        ControlSource control_source,
        ScenePos destination_reached_radius);
    virtual ~DestinationReachedAi() override;
    virtual VehicleAiMoveToStatus move_to(
        const AiWaypoint& ai_waypoint,
        const SkillMap* skills,
        const StaticWorld& world) override;
    virtual std::vector<SkillFactor> skills() const override;
private:
    DestructionFunctionsRemovalTokens on_destroy_rigid_body_;
    RigidBodyVehicle& rigid_body_;
    ControlSource control_source_;
    ScenePos destination_reached_radius_squared_;
};

}
