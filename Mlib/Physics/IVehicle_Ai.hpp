#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Notifier.hpp>
#include <cstddef>
#include <list>
#include <optional>
#include <vector>

namespace Mlib {

struct SkillFactor;
template <typename TData, size_t... tshape>
class FixedArray;
template <class TPoint, class TFlags>
struct PointAndFlags;
enum class WayPointLocation;

enum class VehicleAiMoveToStatus {
	NONE = 0,
	SCENE_VEHICLE_IS_NULL = (1 << 0),
	AUTOPILOT_IS_NULL = (1 << 1),
	WAYPOINT_IS_NAN = (1 << 2),
	POWER_IS_NAN = (1 << 3),
	DESTINATION_REACHED = (1 << 4),
	RESTING_POSITION_REACHED = (1 << 5),
	STOPPED_TO_AVOID_COLLISION = (1 << 6)
};

inline VehicleAiMoveToStatus& operator |= (VehicleAiMoveToStatus& a, VehicleAiMoveToStatus b) {
	(int&)a |= (int)b;
	return a;
}

inline VehicleAiMoveToStatus operator | (VehicleAiMoveToStatus a, VehicleAiMoveToStatus b) {
	return (VehicleAiMoveToStatus)((int)a | (int)b);
}

inline VehicleAiMoveToStatus operator & (VehicleAiMoveToStatus a, VehicleAiMoveToStatus b) {
	return (VehicleAiMoveToStatus)((int)a & (int)b);
}

inline bool any(VehicleAiMoveToStatus a) {
	return a != VehicleAiMoveToStatus::NONE;
}

class IVehicleAi: public virtual DanglingBaseClass, public virtual DestructionNotifier {
public:
	using WayPoint = PointAndFlags<FixedArray<double, 3>, WayPointLocation>;

	virtual ~IVehicleAi() = default;
	virtual VehicleAiMoveToStatus move_to(
		const std::optional<WayPoint>& position_of_destination,
		const std::optional<FixedArray<float, 3>>& velocity_of_destination,
		const std::optional<FixedArray<float, 3>>& velocity_at_destination,
		const std::list<WayPoint>* waypoint_history) = 0;
	virtual std::vector<SkillFactor> skills() const = 0;
};

}
