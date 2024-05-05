#pragma once
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Physics/Ai/IVehicle_Ai.hpp>

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
		float destination_reached_radius);
	virtual ~DestinationReachedAi() override;
	virtual VehicleAiMoveToStatus move_to(
		const std::optional<WayPoint>& position_of_destination,
		const std::optional<FixedArray<float, 3>>& velocity_of_destination,
		const std::optional<FixedArray<float, 3>>& velocity_at_destination,
		const std::list<WayPoint>* waypoint_history,
		const SkillMap* skills) override;
	virtual std::vector<SkillFactor> skills() const override;
private:
	DestructionFunctionsRemovalTokens on_destroy_rigid_body_;
	RigidBodyVehicle& rigid_body_;
	ControlSource control_source_;
	float destination_reached_radius_squared_;
};

}
