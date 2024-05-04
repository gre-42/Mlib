#include "Flying_Missile_Ai.hpp"
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Gravity.hpp>
#include <Mlib/Physics/Rigid_Body/Actor_Type.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Pulses.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Vehicle_Domain.hpp>
#include <Mlib/Physics/Skill_Factor.hpp>
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
	const std::optional<WayPoint>& position_of_destination,
	const std::optional<FixedArray<float, 3>>& velocity_of_destination,
	const std::optional<FixedArray<float, 3>>& velocity_at_destination,
	const std::list<WayPoint>* waypoint_history)
{
	controller_.reset_parameters();
	controller_.reset_relaxation();

	if (!position_of_destination.has_value()) {
		controller_.throttle_engine(0.f, 1.f);
		controller_.set_desired_direction(fixed_zeros<float, 3>(), 1.f);
		controller_.apply();
		return VehicleAiMoveToStatus::WAYPOINT_IS_NAN;
	}
	const auto& waypoint = position_of_destination.value();
	const auto& pod = waypoint.position;
	auto vod = velocity_of_destination.value_or(fixed_zeros<float, 3>());
	controller_.throttle_engine(INFINITY, 1.f);

	auto distance2 = sum(squared(pod - missile_.abs_position()));
	auto eta = std::min(eta_max_, std::sqrt(distance2 / sum(squared(missile_.v_))));
	auto corrected_position_of_destination = pod + eta * vod.casted<double>();

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


std::vector<SkillFactor> FlyingMissileAi::skills() const {
	return { SkillFactor{
		.scenario = SkillScenario{
			.actor_type = ActorType::WINGS,
			.vehicle_domain = VehicleDomain::AIR},
		.factor = 1.f
	}};
}
