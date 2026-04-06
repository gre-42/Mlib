#include "Pathfinding_Waypoints.hpp"
#include <Mlib/Geometry/Graph/Points_And_Adjacency.hpp>
#include <Mlib/Geometry/Primitives/Bvh.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Os/Env.hpp>
#include <Mlib/Physics/Misc/Beacon.hpp>
#include <Mlib/Physics/Physics_Engine/Beacons.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Car_Controllers/Rigid_Body_Vehicle_Controller.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Scene_Graph/Interfaces/Way_Points.hpp>
#include <Mlib/Testing/Assert.hpp>
#include <Mlib/Threads/Throwing_Lock_Guard.hpp>
#include <stdexcept>

using namespace Mlib;

static const auto BEACON = VariableAndHash<std::string>{"beacon"};

PathfindingWaypoints::PathfindingWaypoints(Player& player)
    : player_{ player }
{}

PathfindingWaypoints::~PathfindingWaypoints() = default;

void PathfindingWaypoints::set_waypoint(size_t waypoint_id) {
    if (waypoints_ == nullptr) {
        throw std::runtime_error("Waypoints not set");
    }
    player_.single_waypoint_.set_waypoint(waypoints_->way_points.points.at(waypoint_id), waypoint_id);
}

void PathfindingWaypoints::set_waypoints(std::shared_ptr<const WayPointsAndBvh> waypoints)
{
    waypoints_ = std::move(waypoints);
    // waypoints_bvh_->optimize_search_time(std::cout);
    player_.single_waypoint_.notify_set_waypoints(waypoints_->way_points.points.size());
}

bool PathfindingWaypoints::has_waypoints() const {
    return (waypoints_ != nullptr) && (!waypoints_->way_points.points.empty());
}

void PathfindingWaypoints::select_next_waypoint() {
    ThrowingLockGuard delete_lock{player_.delete_node_mutex_};
    if (!has_waypoints()) {
        return;
    }
    if (getenv_default_bool("DRAW_ALL_WAYPOINTS", false)) {
        if (waypoints_->way_points.points.size() > 30'000) {
            lwarn() << "Refusing to add beacons, number of points is " << waypoints_->way_points.points.size() << " > 30,000";
        } else {
            auto pp = player_.rigid_body()->rbp_.abs_position().casted<CompressedScenePos>();
            for (const auto& p : waypoints_->way_points.points) {
                if (sum(squared(p.position - pp)) > squared(200 * meters)) {
                    continue;
                }
                add_beacon(Beacon::create(funpack(p.position), BEACON));
            }
        }
    }
    assert_true(waypoints_->way_points.adjacency.initialized());
    if (!player_.has_scene_vehicle()) {
        return;
    }
    FixedArray<float, 3> z3 = player_.rigid_body()->rbp_.abs_z();
    FixedArray<ScenePos, 3> pos3 = player_.rigid_body()->rbp_.abs_position();
    if (!player_.single_waypoint_.has_waypoint()) {
        // If we have no current waypoint, find closest point in waypoints array.
        float max_distance = 100;
        size_t closest_id = SIZE_MAX;
        ScenePos closest_distance2 = INFINITY;
        waypoints_->bvh.visit(
            AxisAlignedBoundingBox<CompressedScenePos, 3>::from_center_and_radius(
                pos3.casted<CompressedScenePos>(),
                (CompressedScenePos)max_distance),
            [&](size_t i)
        {
            const auto& rs = waypoints_->way_points.points.at(i).position;
            auto diff = funpack(rs) - pos3;
            auto dist2 = sum(squared(diff));
            if (dist2 < closest_distance2) {
                if ((dist2 < 1e-6) ||
                    (dot0d(diff, z3.casted<ScenePos>()) < -std::cos(45. * degrees) * std::sqrt(dist2)))
                {
                    const auto& neighbors = waypoints_->way_points.adjacency.column(i);
                    assert_true(!neighbors.empty());
                    auto waypoint_has_no_neighbor = [&](){
                        return neighbors.size() == 1;
                    };
                    auto waypoint_has_good_neighbor = [&](){
                        for (const auto& [ni, _] : neighbors) {
                            if (ni == i) {
                                continue;
                            }
                            const auto& nrs = waypoints_->way_points.points.at(ni).position;
                            auto ndiff = funpack(nrs - rs);
                            auto ndist2 = std::sqrt(sum(squared(ndiff)));
                            if ((ndist2 < 1e-12) ||
                                (dot0d(ndiff, z3.casted<ScenePos>()) < -std::cos(45. * degrees) * std::sqrt(ndist2)))
                            {
                                return true;
                            }
                        }
                        return false;
                    };
                    if (waypoint_has_no_neighbor() || waypoint_has_good_neighbor()) {
                        closest_distance2 = dist2;
                        closest_id = i;
                    }
                }
            }
            return true;
        });
        if (closest_id != SIZE_MAX) {
            set_waypoint(closest_id);
        }
    } else {
        if (player_.single_waypoint_.waypoint_reached() &&
            (player_.single_waypoint_.target_waypoint_id() != SIZE_MAX))
        {
            auto deflt = std::chrono::steady_clock::time_point();
            size_t best_id = SIZE_MAX;
            auto best_time = deflt;
            ScenePos best_distance = NAN;
            for (const auto& [r, _] : waypoints_->way_points.adjacency.column(player_.single_waypoint_.target_waypoint_id())) {
                auto candidate_time = player_.single_waypoint_.last_visited(r);
                auto candidate_distance = dot0d(funpack(waypoints_->way_points.points.at(r).position) - pos3, z3.casted<ScenePos>());
                auto r_is_better = [&]() {
                    if (best_id == SIZE_MAX) {
                        return true;
                    }
                    if (best_time != deflt) {
                        if (candidate_time == deflt) {
                            // best_time      != default
                            // candidate_time == default
                            return true;
                        } else {
                            // best_time      != default
                            // candidate_time != default
                            return (candidate_time < best_time);
                        }
                    } else if (candidate_time == deflt) {
                        // best_time      == default
                        // candidate_time == default
                        return (candidate_distance < best_distance);
                    } else {
                        // best_time      == default
                        // candidate_time != default
                        return false;
                    }
                    };
                if (r_is_better()) {
                    best_id = r;
                    best_time = candidate_time;
                    best_distance = candidate_distance;
                }
            }
            if (best_id == SIZE_MAX) {
                throw std::runtime_error("Select next waypoint failed. Forgot diagonal elements of adjacency matrix?");
            }
            set_waypoint(best_id);
        }
    }
}
