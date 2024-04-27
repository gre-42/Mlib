#pragma once
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Physics/IVehicle_Ai.hpp>

namespace Mlib {

class Player;
template <class T>
class DanglingBaseClassRef;

class DriveOrWalkAi final: public IVehicleAi {
	DriveOrWalkAi(const DriveOrWalkAi&) = delete;
	DriveOrWalkAi& operator = (const DriveOrWalkAi&) = delete;
public:
	explicit DriveOrWalkAi(const DanglingBaseClassRef<Player>& player);
	virtual ~DriveOrWalkAi() override;
	virtual VehicleAiMoveToStatus move_to(
		const FixedArray<double, 3>& position_of_destination,
		const FixedArray<float, 3>& velocity_of_destination,
		const std::optional<FixedArray<float, 3>>& velocity_at_destination) override;
	DestructionFunctionsRemovalTokens on_player_delete_externals;
private:
	DanglingBaseClassRef<Player> player_;
};

}
