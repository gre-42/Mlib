#pragma once
#include <Mlib/Players/Vehicle_Ai/IVehicle_Ai.hpp>

namespace Mlib {

class Player;
class RigidBodyVehicle;

class DriveOrWalkAi: public IVehicleAi {
public:
	DriveOrWalkAi(Player& player);
	virtual ~DriveOrWalkAi() override;
	void set_car_controller();
	void set_avatar_controller();
	virtual VehicleAiMoveToStatus move_to(
		const FixedArray<double, 3>& position_of_destination,
		const FixedArray<float, 3>& velocity_of_destination,
		const std::optional<FixedArray<float, 3>>& velocity_at_destination) override;
private:
	Player& player_;
};

}
