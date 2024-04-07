#include "Plane_Ai.hpp"
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Scene_Vehicle/Control_Source.hpp>

using namespace Mlib;

PlaneAi::PlaneAi(Player& player)
	: player_{ player }
{}

PlaneAi::~PlaneAi() = default;

VehicleAiMoveToStatus PlaneAi::move_to(
	const FixedArray<double, 3>& position_of_destination,
	const FixedArray<float, 3>& velocity_of_destination,
	const std::optional<FixedArray<float, 3>>& velocity_at_destination)
{
	if (player_.skills(ControlSource::AI).can_drive) {
		THROW_OR_ABORT("PlaneAi::move_to not yet implemented");
	}
    VehicleAiMoveToStatus result = VehicleAiMoveToStatus::NONE;
    FixedArray<double, 3> pos3 = player_.rigid_body().rbp_.abs_position();
    double distance_to_waypoint2 = sum(squared(pos3 - position_of_destination));
    if (distance_to_waypoint2 < squared(player_.driving_mode().waypoint_reached_radius)) {
        result |= VehicleAiMoveToStatus::DESTINATION_REACHED;
    }
    return result;
}
