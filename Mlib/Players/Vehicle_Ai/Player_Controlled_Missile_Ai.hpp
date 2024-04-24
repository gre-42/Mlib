#pragma once
#include <Mlib/Physics/IVehicle_Ai.hpp>

namespace Mlib {

class Player;

class PlayerControlledMissileAi: public IVehicleAi {
public:
	explicit PlayerControlledMissileAi(Player& player);
	virtual ~PlayerControlledMissileAi() override;
	virtual VehicleAiMoveToStatus move_to(
		const FixedArray<double, 3>& position_of_destination,
		const FixedArray<float, 3>& velocity_of_destination,
		const std::optional<FixedArray<float, 3>>& velocity_at_destination) override;
private:
	Player& player_;
};

}
