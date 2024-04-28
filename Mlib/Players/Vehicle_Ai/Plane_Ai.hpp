#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Physics/IVehicle_Ai.hpp>

namespace Mlib {

class RigidBodyVehicle;

class PlaneAi final: public IVehicleAi {
public:
	explicit PlaneAi(
		const DanglingBaseClassRef<RigidBodyVehicle>& rigid_body,
		const DanglingBaseClassRef<IVehicleAi>& drive_ai,
		const DanglingBaseClassRef<IVehicleAi>& fly_ai);
	virtual ~PlaneAi() override;
	virtual VehicleAiMoveToStatus move_to(
		const FixedArray<double, 3>& position_of_destination,
		const FixedArray<float, 3>& velocity_of_destination,
		const std::optional<FixedArray<float, 3>>& velocity_at_destination) override;
private:
	IVehicleAi& active_ai();
	DestructionFunctionsRemovalTokens on_destroy_rigid_body_;
	DestructionFunctionsRemovalTokens on_destroy_drive_ai_;
	DestructionFunctionsRemovalTokens on_destroy_fly_ai_;
	DanglingBaseClassRef<RigidBodyVehicle> rigid_body_;
	DanglingBaseClassRef<IVehicleAi> drive_ai_;
	DanglingBaseClassRef<IVehicleAi> fly_ai_;
};

}
