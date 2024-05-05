#include "Follower_Ai.hpp"
#include <Mlib/Geometry/Mesh/Point_And_Flags.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Ai/IVehicle_Ai.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene_Graph/Way_Point_Location.hpp>

using namespace Mlib;

FollowerAi::FollowerAi(
	AdvanceTimes& advance_times,
	const DanglingBaseClassRef<RigidBodyVehicle>& follower,
	const DanglingBaseClassRef<RigidBodyVehicle>& followed)
	: advance_times_{ advance_times }
	, follower_{ follower }
	, followed_{ followed.ptr() }
	, follower_on_destroy_{ follower->on_destroy, CURRENT_SOURCE_LOCATION }
	, followed_on_destroy_{ followed->on_destroy, CURRENT_SOURCE_LOCATION }
{
	follower_on_destroy_.add([this]() { global_object_pool.remove(this); }, CURRENT_SOURCE_LOCATION);
	followed_on_destroy_.add([this]() { followed_ = nullptr; }, CURRENT_SOURCE_LOCATION);
	advance_times_.add_advance_time({ *this, CURRENT_SOURCE_LOCATION }, CURRENT_SOURCE_LOCATION);
}

FollowerAi::~FollowerAi() {
	on_destroy.clear();
}

void FollowerAi::advance_time(float dt, std::chrono::steady_clock::time_point time) {
	if (followed_ != nullptr) {
		follower_->move_to(
			RigidBodyVehicle::WayPoint{ followed_->abs_com(), WayPointLocation::UNKNOWN },
			followed_->rbp_.v_,
			std::nullopt,
			nullptr,
			nullptr);
	}
}
