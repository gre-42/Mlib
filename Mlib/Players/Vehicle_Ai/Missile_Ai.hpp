#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Players/Vehicle_Ai/IVehicle_Ai.hpp>
#include <Mlib/Signal/Pid_Controller.hpp>

namespace Mlib {

class RigidBodyMissileController;
class RigidBodyPulses;

class MissileAi: public IVehicleAi {
public:
	explicit MissileAi(
		const PidController<FixedArray<float, 3>, float>& pid,
		RigidBodyMissileController& controller,
		RigidBodyPulses& missile,
		RigidBodyPulses& target,
		float destination_reached_radius);
	virtual ~MissileAi() override;
	virtual VehicleAiMoveToStatus move_to(
		const FixedArray<double, 3>& destination_position,
		const std::optional<FixedArray<float, 3>>& destination_velocity) override;
private:
	PidController<FixedArray<float, 3>, float> pid_;
	float destination_reached_radius_squared_;
	RigidBodyMissileController& controller_;
	RigidBodyPulses& missile_;
	RigidBodyPulses& target_;
};

}
