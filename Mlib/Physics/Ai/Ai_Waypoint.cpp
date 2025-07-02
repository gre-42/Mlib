#include "Ai_Waypoint.hpp"
#include <Mlib/Geometry/Graph/Point_And_Flags.hpp>
#include <Mlib/Geometry/Intersection/Ray_Sphere_Intersection.hpp>
#include <Mlib/Geometry/Ray_Segment_3D.hpp>
#include <Mlib/Scene_Graph/Way_Point_Location.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

AiWaypoint::AiWaypoint(
    const std::optional<WayPoint>& position_of_destination,
    const std::optional<FixedArray<float, 3>>& velocity_of_destination,
    const std::optional<FixedArray<float, 3>>& velocity_at_destination,
    const std::list<WayPoint>* waypoint_history)
    : position_of_destination_{ position_of_destination }
    , velocity_of_destination_{ velocity_of_destination }
    , velocity_at_destination_{ velocity_at_destination }
    , waypoint_history_{ waypoint_history }
{}

FixedArray<CompressedScenePos, 3> AiWaypoint::interpolated_position(
    const FixedArray<CompressedScenePos, 3>& vehicle_position,
    ScenePos radius_squared,
    CompressedScenePos dy) const
{
    if (!position_of_destination_.has_value()) {
        THROW_OR_ABORT("Position of desination is undefined");
    }
    auto& pod = position_of_destination_->position;
    FixedArray<CompressedScenePos, 3> dy3{ (CompressedScenePos)0.f, dy, (CompressedScenePos)0.f };
    if ((waypoint_history_ == nullptr) ||
        waypoint_history_->empty())
    {
        return pod + dy3;
    }
    RaySegment3D<ScenePos, CompressedScenePos> rs(waypoint_history_->rbegin()->position, pod);
    ScenePos lambda;
    if (!ray_intersects_sphere(
        funpack(rs.start),
        funpack(rs.direction),
        funpack(vehicle_position),
        radius_squared,
        &lambda,
        (ScenePos*)nullptr))
    {
        return pod + dy3;
    } else {
        if ((lambda >= 0) && (lambda < rs.length)) {
            return rs.start + (rs.direction * lambda).casted<CompressedScenePos>() + dy3;
        } else {
            return pod + dy3;
        }
    }
    return pod + dy3;
}

bool AiWaypoint::has_position_of_destination() const {
    return position_of_destination_.has_value();
}

FixedArray<CompressedScenePos, 3> AiWaypoint::position_of_destination(CompressedScenePos dy) const {
    if (!position_of_destination_.has_value()) {
        THROW_OR_ABORT("Position of desintation not defined");
    }
    auto res = position_of_destination_->position;
    res(1) += dy;
    return res;
}

WayPointLocation AiWaypoint::flags() const {
    if (!position_of_destination_.has_value()) {
        THROW_OR_ABORT("Position of desintation not defined");
    }
    return position_of_destination_->flags;
}

FixedArray<float, 3> AiWaypoint::velocity_of_destination(const FixedArray<float, 3>& deflt) const {
    return velocity_of_destination_.value_or(deflt);
}

FixedArray<float, 3> AiWaypoint::velocity_at_destination(const FixedArray<float, 3>& deflt) const {
    return velocity_at_destination_.value_or(deflt);
}

WayPointLocation AiWaypoint::latest_history_flags() const {
    if (waypoint_history_ == nullptr) {
        THROW_OR_ABORT("Waypoint history is null");
    }
    if (waypoint_history_->empty()) {
        return WayPointLocation::NONE;
    }
    return waypoint_history_->rbegin()->flags;
}

bool AiWaypoint::has_velocity_at_destination() const {
    return velocity_at_destination_.has_value();
}

FixedArray<float, 3> AiWaypoint::velocity_at_destination() const {
    if (!velocity_at_destination_.has_value()) {
        THROW_OR_ABORT("Velocity at destination is undefined");
    }
    return *velocity_at_destination_;
}
