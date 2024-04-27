#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Physics/IVehicle_Ai.hpp>

namespace Mlib {

class RigidBodyVehicle;

class PlaneAi final: public IVehicleAi {
public:
	explicit PlaneAi(
		const DanglingBaseClassRef<RigidBodyVehicle>& rigid_body,
		std::unique_ptr<IVehicleAi>&& drive_ai,
		std::unique_ptr<IVehicleAi>&& fly_ai);
	virtual ~PlaneAi() override;
	virtual VehicleAiMoveToStatus move_to(
		const FixedArray<double, 3>& position_of_destination,
		const FixedArray<float, 3>& velocity_of_destination,
		const std::optional<FixedArray<float, 3>>& velocity_at_destination) override;
private:
	IVehicleAi& active_ai();
	DanglingBaseClassRef<RigidBodyVehicle> rigid_body_;
	std::unique_ptr<IVehicleAi> drive_ai_;
	std::unique_ptr<IVehicleAi> fly_ai_;
};

}
