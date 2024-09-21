#include "Pathfinding_Waypoints.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Env.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Mesh/Points_And_Adjacency.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Physics/Misc/Beacon.hpp>
#include <Mlib/Physics/Physics_Engine/Beacons.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Car_Controllers/Rigid_Body_Vehicle_Controller.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

PathfindingWaypoints::PathfindingWaypoints(
    Player& player,
    const PhysicsEngineConfig& cfg)
    : player_{ player }
    , cfg_{ cfg }
{}

PathfindingWaypoints::~PathfindingWaypoints() = default;

void PathfindingWaypoints::set_waypoint(size_t waypoint_id) {
    player_.single_waypoint_.set_waypoint(waypoints_->points.at(waypoint_id), waypoint_id);
}

void PathfindingWaypoints::set_waypoints(const PointsAndAdjacencyResource& waypoints)
{
    waypoints_bvh_ = std::make_unique<Bvh<ScenePos, size_t, 3>>(
        FixedArray<ScenePos, 3>{cfg_.bvh_max_size, cfg_.bvh_max_size, cfg_.bvh_max_size},
        cfg_.bvh_levels);
    waypoints_ = std::make_unique<PointsAndAdjacencyResource>(waypoints);
    for (const auto& [i, p] : enumerate(waypoints.points)) {
        waypoints_bvh_->insert(AxisAlignedBoundingBox<ScenePos, 3>::from_point(p.position), i);
    }
    // waypoints_bvh_->optimize_search_time(std::cout);
    player_.single_waypoint_.notify_set_waypoints(waypoints_->points.size());
}

bool PathfindingWaypoints::has_waypoints() const {
    return (waypoints_ != nullptr) && (!waypoints_->points.empty());
}

void PathfindingWaypoints::select_next_waypoint() {
    player_.delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!has_waypoints()) {
        return;
    }
    if (getenv_default_bool("DRAW_ALL_WAYPOINTS", false)) {
        if (waypoints_->points.size() > 30'000) {
            lwarn() << "Refusing to add beacons, number of points is " << waypoints_->points.size() << " > 30,000";
        } else {
            auto pp = player_.rigid_body().rbp_.abs_position();
            for (const auto& p : waypoints_->points) {
                if (sum(squared(p.position - pp)) > squared(200 * meters)) {
                    continue;
                }
                add_beacon(Beacon::create(p, "beacon"));
            }
        }
    }
    assert_true(waypoints_->adjacency.initialized());
    if (!player_.has_scene_vehicle()) {
        return;
    }
    FixedArray<float, 3> z3 = player_.rigid_body().rbp_.abs_z();
    FixedArray<ScenePos, 3> pos3 = player_.rigid_body().rbp_.abs_position();
    if (!player_.single_waypoint_.has_waypoint()) {
        // If we have no current waypoint, find closest point in waypoints array.
        float max_distance = 100;
        size_t closest_id = SIZE_MAX;
        ScenePos closest_distance2 = INFINITY;
        waypoints_bvh_->visit(
            AxisAlignedBoundingBox<ScenePos, 3>::from_center_and_radius(pos3, max_distance),
            [&](size_t i)
        {
            const auto& rs = waypoints_->points.at(i).position;
            auto diff = rs - pos3;
            auto dist2 = sum(squared(diff));
            if ((dist2 < 1e-6) ||
                (dot0d(diff / std::sqrt(dist2), z3.casted<ScenePos>()) < -std::cos(45. * degrees)))
            {
                if (dist2 < closest_distance2) {
                    closest_distance2 = dist2;
                    closest_id = i;
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
            for (const auto& [r, _] : waypoints_->adjacency.column(player_.single_waypoint_.target_waypoint_id())) {
                auto candidate_time = player_.single_waypoint_.last_visited(r);
                auto candidate_distance = dot0d(waypoints_->points.at(r).position - pos3, z3.casted<ScenePos>());
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
                THROW_OR_ABORT("Select next waypoint failed. Forgot diagonal elements of adjacency matrix?");
            }
            set_waypoint(best_id);
        }
    }
}
