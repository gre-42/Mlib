#pragma once
#include <Mlib/Scene_Config/Scene_Precision.hpp>
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

class AiWaypoint {
public:
    using WayPoint = PointAndFlags<FixedArray<CompressedScenePos, 3>, WayPointLocation>;

    AiWaypoint(
        const std::optional<WayPoint>& position_of_destination,
        const std::optional<FixedArray<float, 3>>& velocity_of_destination,
        const std::optional<FixedArray<float, 3>>& velocity_at_destination,
        const std::list<WayPoint>* waypoint_history);
    bool has_position_of_destination() const;
    FixedArray<CompressedScenePos, 3> position_of_destination(CompressedScenePos dy) const;
    WayPointLocation flags() const;
    WayPointLocation latest_history_flags() const;
    bool has_velocity_at_destination() const;
    FixedArray<float, 3> velocity_at_destination() const;
    FixedArray<float, 3> velocity_of_destination(const FixedArray<float, 3>& deflt) const;
    FixedArray<float, 3> velocity_at_destination(const FixedArray<float, 3>& deflt) const;
    FixedArray<CompressedScenePos, 3> interpolated_position(
        const FixedArray<CompressedScenePos, 3>& vehicle_position,
        ScenePos radius_squared,
        CompressedScenePos dy) const;

private:
    const std::optional<WayPoint>& position_of_destination_;
    const std::optional<FixedArray<float, 3>>& velocity_of_destination_;
    const std::optional<FixedArray<float, 3>>& velocity_at_destination_;
    const std::list<WayPoint>* waypoint_history_;
};

}
