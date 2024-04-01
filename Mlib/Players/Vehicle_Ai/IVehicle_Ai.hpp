#pragma once
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

enum class VehicleAiMoveToStatus {
	NONE = 0,
	SCENE_VEHICLE_IS_NULL = (1 << 0),
	SKILL_MISSING = (1 << 1),
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

class IVehicleAi {
public:
	virtual ~IVehicleAi() = default;
	virtual VehicleAiMoveToStatus move_to(
		const FixedArray<double, 3>& destination_position,
		const FixedArray<float, 3>& destination_velocity) = 0;
};

}
