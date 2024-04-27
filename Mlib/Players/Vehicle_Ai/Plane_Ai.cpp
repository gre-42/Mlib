#include "Plane_Ai.hpp"
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Vehicle_Domain.hpp>

using namespace Mlib;

PlaneAi::PlaneAi(
	const DanglingBaseClassRef<RigidBodyVehicle>& rigid_body,
	std::unique_ptr<IVehicleAi>&& drive_ai,
	std::unique_ptr<IVehicleAi>&& fly_ai)
	: rigid_body_{ rigid_body }
	, drive_ai_{ std::move(drive_ai) }
	, fly_ai_{ std::move(fly_ai) }
{}

PlaneAi::~PlaneAi() {
	on_destroy.clear();
}

VehicleAiMoveToStatus PlaneAi::move_to(
	const FixedArray<double, 3>& position_of_destination,
	const FixedArray<float, 3>& velocity_of_destination,
	const std::optional<FixedArray<float, 3>>& velocity_at_destination)
{
	return active_ai().move_to(
		position_of_destination,
		velocity_of_destination,
		velocity_at_destination);
}

IVehicleAi& PlaneAi::active_ai() {
	switch (rigid_body_->current_vehicle_domain_) {
	case VehicleDomain::AIR:
		return *fly_ai_;
	case VehicleDomain::GROUND:
		return *drive_ai_;
	case VehicleDomain::UNDEFINED:
		THROW_OR_ABORT("Vehicle \"" + rigid_body_->name() + "\": Undefined vehicle domain");
	}
	THROW_OR_ABORT("Vehicle \"" + rigid_body_->name() + "\": Unknown vehicle domain");
}
