#include "Vehicle_Ai_Advance_Time.hpp"
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Players/Vehicle_Ai/IVehicle_Ai.hpp>

using namespace Mlib;

VehicleAiAdvanceTime::VehicleAiAdvanceTime(
	AdvanceTimes& advance_times,
	std::unique_ptr<IVehicleAi>&& vehicle_ai,
	DanglingBaseClassRef<RigidBodyVehicle> follower,
	DanglingBaseClassRef<RigidBodyVehicle> followed)
	: shutting_down_{ false }
	, advance_times_{ advance_times }
	, vehicle_ai_{ std::move(vehicle_ai) }
	, follower_{ follower }
	, followed_{ followed.ptr() }
	, follower_on_destroy_{ follower->on_destroy }
	, followed_on_destroy_{ followed->on_destroy }
{
	follower_on_destroy_.add([this]() { if (!shutting_down_) { delete this; }});
	followed_on_destroy_.add([this]() { followed_ = nullptr; });
	advance_times_.add_advance_time(*this);
	dgs_.add([this]() { advance_times_.delete_advance_time(*this, CURRENT_SOURCE_LOCATION); });
}

VehicleAiAdvanceTime::~VehicleAiAdvanceTime() {
	shutting_down_ = true;
}

void VehicleAiAdvanceTime::advance_time(float dt, std::chrono::steady_clock::time_point time) {
	if (followed_ != nullptr) {
		vehicle_ai_->move_to(followed_->abs_com(), followed_->rbp_.v_, std::nullopt);
	}
}
