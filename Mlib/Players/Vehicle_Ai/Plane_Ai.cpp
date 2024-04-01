#include "Plane_Ai.hpp"

using namespace Mlib;

PlaneAi::PlaneAi(Player& player)
	: player_{ player }
{}

PlaneAi::~PlaneAi() = default;

VehicleAiMoveToStatus PlaneAi::move_to(
	const FixedArray<double, 3>& destination_position,
	const FixedArray<float, 3>& destination_velocity)
{
	THROW_OR_ABORT("PlaneAi::move_to not yet implemented");
}
