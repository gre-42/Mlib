#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Physics/IVehicle_Ai.hpp>
#include <Mlib/Signal/Pid_Controller.hpp>

namespace Mlib {

class RigidBodyMissileController;
class RigidBodyPulses;

class AutonomousMissileAi: public IVehicleAi {
	AutonomousMissileAi(const AutonomousMissileAi&) = delete;
	AutonomousMissileAi& operator = (const AutonomousMissileAi&) = delete;
public:
	explicit AutonomousMissileAi(
		const PidController<FixedArray<float, 3>, float>& pid,
		Interp<float, float> dy,
		double eta_max,
		RigidBodyMissileController& controller,
		RigidBodyPulses& missile,
		float destination_reached_radius);
	virtual ~AutonomousMissileAi() override;
	virtual VehicleAiMoveToStatus move_to(
		const FixedArray<double, 3>& position_of_destination,
		const FixedArray<float, 3>& velocity_of_destination,
		const std::optional<FixedArray<float, 3>>& velocity_at_destination) override;
private:
	PidController<FixedArray<float, 3>, float> pid_;
	Interp<float, float> dy_;
	double eta_max_;
	float destination_reached_radius_squared_;
	RigidBodyMissileController& controller_;
	RigidBodyPulses& missile_;
};

}
