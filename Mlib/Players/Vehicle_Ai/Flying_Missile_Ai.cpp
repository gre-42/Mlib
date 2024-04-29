#include "Flying_Missile_Ai.hpp"
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Gravity.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Pulses.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Missile_Controllers/Rigid_Body_Missile_Controller.hpp>

using namespace Mlib;

FlyingMissileAi::FlyingMissileAi(
	RigidBodyVehicle& rigid_body,
	const PidController<FixedArray<float, 3>, float>& pid,
	Interp<float, float> dy,
	double eta_max,
	RigidBodyMissileController& controller,
	RigidBodyPulses& missile,
	float destination_reached_radius)
	: on_destroy_rigid_body_{ rigid_body.on_destroy, CURRENT_SOURCE_LOCATION }
	, pid_{ pid }
	, dy_{ std::move(dy) }
	, eta_max_{ eta_max }
	, destination_reached_radius_squared_{ squared(destination_reached_radius) }
	, controller_{ controller }
	, missile_{ missile }
{
	on_destroy_rigid_body_.add([this]() { global_object_pool.remove(this); }, CURRENT_SOURCE_LOCATION);
}

FlyingMissileAi::~FlyingMissileAi() {
	on_destroy.clear();
}

VehicleAiMoveToStatus FlyingMissileAi::move_to(
	const FixedArray<double, 3>& position_of_destination,
	const FixedArray<float, 3>& velocity_of_destination,
	const std::optional<FixedArray<float, 3>>& velocity_at_destination)
{
	controller_.reset_parameters();
	controller_.reset_relaxation();

	if (any(isnan(position_of_destination))) {
		controller_.throttle_engine(0.f, 1.f);
		controller_.set_desired_direction(fixed_zeros<float, 3>(), 1.f);
		controller_.apply();
		return VehicleAiMoveToStatus::WAYPOINT_IS_NAN;
	}
	controller_.throttle_engine(INFINITY, 1.f);

	auto distance2 = sum(squared(position_of_destination - missile_.abs_position()));
	auto eta = std::min(eta_max_, std::sqrt(distance2 / sum(squared(missile_.v_))));
	auto corrected_position_of_destination = position_of_destination + eta * velocity_of_destination.casted<double>();

	auto dir_d = corrected_position_of_destination - missile_.abs_position();
	auto l2_d = sum(squared(dir_d));
	if (l2_d < destination_reached_radius_squared_) {
		return VehicleAiMoveToStatus::DESTINATION_REACHED;
	}
	auto dir = (dir_d / std::sqrt(l2_d)).casted<float>();

	auto vz = dot0d(missile_.v_, missile_.rotation_.column(2));
	if (vz <= 0.f) {
		dir -= gravity_direction * dy_(-vz);
		auto l = std::sqrt(sum(squared(dir)));
		if (l < 1e-12) {
			THROW_OR_ABORT("Direction length too small, please choose a smaller dy value");
		}
		dir /= l;
	}

	controller_.set_desired_direction(pid_(dir), 1.f);
	controller_.apply();
	return VehicleAiMoveToStatus::NONE;
}
