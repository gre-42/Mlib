#include "Missile_Ai.hpp"
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Pulses.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Missile_Controllers/Rigid_Body_Missile_Controller.hpp>

using namespace Mlib;

MissileAi::MissileAi(
	const PidController<FixedArray<float, 3>, float>& pid,
	RigidBodyMissileController& controller,
	RigidBodyPulses& missile,
	RigidBodyPulses& target,
	float destination_reached_radius)
	: pid_{ pid }
	, destination_reached_radius_squared_{ squared(destination_reached_radius) }
	, controller_{ controller }
	, missile_{ missile }
	, target_{ target }
{}

MissileAi::~MissileAi() = default;

VehicleAiMoveToStatus MissileAi::move_to(
	const FixedArray<double, 3>& destination_position,
	const std::optional<FixedArray<float, 3>>& destination_velocity)
{
	auto dir = destination_position - missile_.abs_position();
	auto l2 = sum(squared(dir));
	if (l2 < destination_reached_radius_squared_) {
		return VehicleAiMoveToStatus::DESTINATION_REACHED;
	}
	dir /= std::sqrt(l2);
	controller_.reset_parameters();
	controller_.reset_relaxation();
	controller_.set_desired_direction(pid_(dir.casted<float>()), 1.f);
	return VehicleAiMoveToStatus::NONE;
}
