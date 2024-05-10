#pragma once
#include <cstddef>
#include <list>
#include <optional>

namespace Mlib {

struct WayPoint;
template <typename TData, size_t... tshape>
class FixedArray;
template <class TPoint, class TFlags>
struct PointAndFlags;
enum class WayPointLocation;

struct AiWaypoint {
	using WayPoint = PointAndFlags<FixedArray<double, 3>, WayPointLocation>;

	const std::optional<WayPoint>& position_of_destination;
	const std::optional<FixedArray<float, 3>>& velocity_of_destination;
	const std::optional<FixedArray<float, 3>>& velocity_at_destination;
	const std::list<WayPoint>* waypoint_history;
	FixedArray<double, 3> interpolated_position(
		const FixedArray<double, 3>& vehicle_position,
		double radius_squared) const;
};

}
