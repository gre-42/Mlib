#pragma once
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Physics/Ai/IVehicle_Ai.hpp>

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
		const AiWaypoint& ai_waypoint,
		const SkillMap* skills) override;
	virtual std::vector<SkillFactor> skills() const override;
private:
	DestructionFunctionsRemovalTokens on_player_delete_externals_;
	DanglingBaseClassRef<Player> player_;
};

}
