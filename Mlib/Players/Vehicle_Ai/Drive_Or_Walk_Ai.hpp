#pragma once
#include <Mlib/Physics/IVehicle_Ai.hpp>

namespace Mlib {

class Player;

class DriveOrWalkAi: public IVehicleAi {
	DriveOrWalkAi(const DriveOrWalkAi&) = delete;
	DriveOrWalkAi& operator = (const DriveOrWalkAi&) = delete;
public:
	explicit DriveOrWalkAi(Player& player);
	virtual ~DriveOrWalkAi() override;
	virtual VehicleAiMoveToStatus move_to(
		const FixedArray<double, 3>& position_of_destination,
		const FixedArray<float, 3>& velocity_of_destination,
		const std::optional<FixedArray<float, 3>>& velocity_at_destination) override;
private:
	Player& player_;
};

}
