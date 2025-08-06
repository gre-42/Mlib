#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Physics/Ai/IVehicle_Ai.hpp>

namespace Mlib {

class RigidBodyMissileController;
class RigidBodyPulses;
class RigidBodyVehicle;

class FlyingMissileAi final: public IVehicleAi {
    FlyingMissileAi(const FlyingMissileAi&) = delete;
    FlyingMissileAi& operator = (const FlyingMissileAi&) = delete;
public:
    explicit FlyingMissileAi(
        RigidBodyVehicle& rigid_body,
        Interp<float, float> dy,
        double eta_max,
        RigidBodyMissileController& controller,
        float waypoint_reached_radius,
        float resting_position_reached_radius,
        float maximum_velocity);
    virtual ~FlyingMissileAi() override;
    virtual VehicleAiMoveToStatus move_to(
        const AiWaypoint& ai_waypoint,
        const SkillMap* skills,
        const StaticWorld& world,
        float dt) override;
    virtual std::vector<SkillFactor> skills() const override;
private:
    DestructionFunctionsRemovalTokens on_destroy_rigid_body_;
    Interp<float, float> dy_;
    double eta_max_;
    float waypoint_reached_radius_squared_;
    float resting_position_reached_radius_squared_;
    RigidBodyMissileController& controller_;
    RigidBodyVehicle& rigid_body_;
    float maximum_velocity_;
};

}
