#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Memory/Destruction_Guards.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <memory>

namespace Mlib {

class IVehicleAi;
class RigidBodyVehicle;
class AdvanceTimes;

class VehicleAiAdvanceTime: public IAdvanceTime {
public:
	explicit VehicleAiAdvanceTime(
		AdvanceTimes& advance_times,
		std::unique_ptr<IVehicleAi> vehicle_ai,
		DanglingBaseClassRef<RigidBodyVehicle> follower,
		DanglingBaseClassRef<RigidBodyVehicle> followed);
	virtual ~VehicleAiAdvanceTime();
	virtual void advance_time(float dt, std::chrono::steady_clock::time_point time);
private:
	bool shutting_down_;
	AdvanceTimes& advance_times_;
	std::unique_ptr<IVehicleAi> vehicle_ai_;
	DanglingBaseClassRef<RigidBodyVehicle> follower_;
	DanglingBaseClassPtr<RigidBodyVehicle> followed_;
	DestructionGuards dgs_;
	DestructionFunctionsRemovalTokens follower_on_destroy_;
	DestructionFunctionsRemovalTokens followed_on_destroy_;
};

}
