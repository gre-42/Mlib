#include "Player_Controlled_Missile_Ai.hpp"
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Scene_Vehicle/Control_Source.hpp>

using namespace Mlib;

PlayerControlledMissileAi::PlayerControlledMissileAi(Player& player)
	: player_{ player }
{}

PlayerControlledMissileAi::~PlayerControlledMissileAi() = default;

VehicleAiMoveToStatus PlayerControlledMissileAi::move_to(
	const FixedArray<double, 3>& position_of_destination,
	const FixedArray<float, 3>& velocity_of_destination,
	const std::optional<FixedArray<float, 3>>& velocity_at_destination)
{
    if (!player_.has_scene_vehicle()) {
        return VehicleAiMoveToStatus::SCENE_VEHICLE_IS_NULL;
    }
    if (!player_.skills(ControlSource::AI).can_drive) {
        return VehicleAiMoveToStatus::SKILL_MISSING;
    }
    auto& player_rb = player_.rigid_body();
    if (player_rb.autonomous_missile_ai_ == nullptr) {
        return VehicleAiMoveToStatus::AUTOPILOT_IS_NULL;
    }
    return player_rb.autonomous_missile_ai_->move_to(
        position_of_destination,
        velocity_of_destination,
        velocity_at_destination);
}
