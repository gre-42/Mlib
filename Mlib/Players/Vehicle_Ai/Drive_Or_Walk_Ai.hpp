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
		const std::optional<WayPoint>& position_of_destination,
		const std::optional<FixedArray<float, 3>>& velocity_of_destination,
		const std::optional<FixedArray<float, 3>>& velocity_at_destination,
		const std::list<WayPoint>* waypoint_history) override;
	virtual std::vector<SkillFactor> skills() const override;
private:
	DestructionFunctionsRemovalTokens on_player_delete_externals_;
	DanglingBaseClassRef<Player> player_;
};

}
