#pragma once
#include <Mlib/Players/Vehicle_Ai/IVehicle_Ai.hpp>

namespace Mlib {

class Player;

class PlaneAi: public IVehicleAi {
public:
	explicit PlaneAi(Player& player);
	virtual ~PlaneAi() override;
	virtual VehicleAiMoveToStatus move_to(
		const FixedArray<double, 3>& position_of_destination,
		const FixedArray<float, 3>& velocity_of_destination,
		const std::optional<FixedArray<float, 3>>& velocity_at_destination) override;
private:
	Player& player_;
};

}
