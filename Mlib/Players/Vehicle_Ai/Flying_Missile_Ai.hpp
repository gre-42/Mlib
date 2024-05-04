#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Physics/IVehicle_Ai.hpp>
#include <Mlib/Signal/Pid_Controller.hpp>

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
		const PidController<FixedArray<float, 3>, float>& pid,
		Interp<float, float> dy,
		double eta_max,
		RigidBodyMissileController& controller,
		RigidBodyPulses& missile,
		float destination_reached_radius);
	virtual ~FlyingMissileAi() override;
	virtual VehicleAiMoveToStatus move_to(
		const std::optional<WayPoint>& position_of_destination,
		const std::optional<FixedArray<float, 3>>& velocity_of_destination,
		const std::optional<FixedArray<float, 3>>& velocity_at_destination,
		const std::list<WayPoint>* waypoint_history) override;
	virtual std::vector<SkillFactor> skills() const override;
private:
	DestructionFunctionsRemovalTokens on_destroy_rigid_body_;
	PidController<FixedArray<float, 3>, float> pid_;
	Interp<float, float> dy_;
	double eta_max_;
	float destination_reached_radius_squared_;
	RigidBodyMissileController& controller_;
	RigidBodyPulses& missile_;
};

}
