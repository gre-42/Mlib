#include "Destination_Reached_Ai.hpp"
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Ai/Skill_Factor.hpp>
#include <Mlib/Physics/Ai/Skill_Map.hpp>
#include <Mlib/Physics/Ai/Skills.hpp>
#include <Mlib/Physics/Rigid_Body/Actor_Type.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Pulses.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Vehicle_Domain.hpp>

using namespace Mlib;

DestinationReachedAi::DestinationReachedAi(
	RigidBodyVehicle& rigid_body,
	ControlSource control_source,
	float destination_reached_radius)
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
	const std::optional<WayPoint>& position_of_destination,
	const std::optional<FixedArray<float, 3>>& velocity_of_destination,
	const std::optional<FixedArray<float, 3>>& velocity_at_destination,
	const std::list<WayPoint>* waypoint_history,
	const SkillMap* skills)
{
	if ((skills != nullptr) && !skills->skills(control_source_).can_drive) {
		return VehicleAiMoveToStatus::SKILL_IS_MISSING;
	}
	if (!position_of_destination.has_value()) {
		return VehicleAiMoveToStatus::WAYPOINT_IS_NAN;
	}
	const auto& waypoint = position_of_destination.value();
	const auto& pod = waypoint.position;

	auto distance2 = sum(squared(pod - rigid_body_.rbp_.abs_position()));

	if (distance2 < destination_reached_radius_squared_) {
		return VehicleAiMoveToStatus::DESTINATION_REACHED;
	}
	return VehicleAiMoveToStatus::NONE;
}


std::vector<SkillFactor> DestinationReachedAi::skills() const {
	return {
		SkillFactor{
			.scenario = SkillScenario{
				.actor_type = ActorType::DESTINATION_REACHED_STATUS,
				.vehicle_domain = VehicleDomain::AIR},
			.factor = 1.f},
		SkillFactor{
			.scenario = SkillScenario{
				.actor_type = ActorType::DESTINATION_REACHED_STATUS,
				.vehicle_domain = VehicleDomain::GROUND},
			.factor = 1.f}
	};
}
