#pragma once
#include <Mlib/Players/Vehicle_Ai/IVehicle_Ai.hpp>

namespace Mlib {

class Player;

class PlaneAi: public IVehicleAi {
public:
	explicit PlaneAi(Player& player);
	virtual ~PlaneAi() override;
	virtual VehicleAiMoveToStatus move_to(
		const FixedArray<double, 3>& destination_position,
		const FixedArray<float, 3>& destination_velocity) override;
private:
	Player& player_;
};

}
