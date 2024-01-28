#include "Pathfinding_Waypoints.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Mesh/Points_And_Adjacency.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
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
: player_{player},
  cfg_{cfg}
{}

PathfindingWaypoints::~PathfindingWaypoints()
{}

void PathfindingWaypoints::set_waypoint(size_t waypoint_id) {
    player_.single_waypoint_.set_waypoint(waypoints_->points.at(waypoint_id), waypoint_id);
}

void PathfindingWaypoints::set_waypoints(const PointsAndAdjacency<double, 3>& waypoints)
{
    waypoints_bvh_ = std::make_unique<Bvh<double, size_t, 3>>(
        FixedArray<double, 3>{cfg_.bvh_max_size, cfg_.bvh_max_size, cfg_.bvh_max_size},
        cfg_.bvh_levels);
    waypoints_ = std::make_unique<PointsAndAdjacency<double, 3>>(waypoints);
    size_t i = 0;
    for (const FixedArray<double, 3>& p : waypoints.points) {
        waypoints_bvh_->insert(p, i++);
    }
    // waypoints_bvh_->optimize_search_time(std::cout);
    player_.single_waypoint_.notify_set_waypoints(waypoints_->points.size());
}

bool PathfindingWaypoints::has_waypoints() const {
    player_.delete_node_mutex_.notify_reading();
    return (waypoints_ != nullptr) && (!waypoints_->points.empty());
}

void PathfindingWaypoints::select_next_waypoint() {
    player_.delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!has_waypoints()) {
        return;
    }
    assert_true(waypoints_->adjacency.initialized());
    if (!player_.has_scene_vehicle()) {
        return;
    }
    FixedArray<float, 3> z3 = player_.rigid_body().rbp_.abs_z();
    FixedArray<double, 3> pos3 = player_.rigid_body().rbp_.abs_position();
    if (player_.single_waypoint_.waypoint_id_ == SIZE_MAX) {
        // If we have no current waypoint, find closest point in waypoints array.
        float max_distance = 100;
        size_t closest_id = SIZE_MAX;
        double closest_distance2 = INFINITY;
        waypoints_bvh_->visit(
            {pos3, max_distance},
            [&](size_t i)
        {
            const auto& rs = waypoints_->points.at(i);
            auto diff = rs - pos3;
            auto dist2 = sum(squared(diff));
            if ((dist2 < 1e-6) ||
                (dot0d(diff / std::sqrt(dist2), z3.casted<double>()) < -std::cos(45. * degrees)))
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
        if (player_.single_waypoint_.waypoint_reached_) {
            if (player_.single_waypoint_.nwaypoints_reached_ < 2) {
                // If we already have less than two waypoints, go further forward.
                size_t best_id = SIZE_MAX;
                double best_distance = INFINITY;
                for (const auto& rs : waypoints_->adjacency.column(player_.single_waypoint_.waypoint_id_)) {
                    double dist = dot0d(waypoints_->points.at(rs.first) - pos3, z3.casted<double>());
                    if (dist < best_distance) {
                        best_id = rs.first;
                        best_distance = dist;
                    }
                }
                if (best_id == SIZE_MAX) {
                    THROW_OR_ABORT("Select next waypoint failed. Forgot diagonal elements of adjacency matrix?");
                }
                set_waypoint(best_id);
            } else {
                // If we already have two waypoints, pick oldest neighbor.
                auto deflt = std::chrono::steady_clock::time_point();
                size_t best_id = SIZE_MAX;
                auto best_time = deflt;
                for (const auto& rs : waypoints_->adjacency.column(player_.single_waypoint_.waypoint_id_)) {
                    if ((best_id == SIZE_MAX) ||
                        (player_.single_waypoint_.last_visited_.at(rs.first) == deflt) ||
                        ((best_time != deflt) && (player_.single_waypoint_.last_visited_.at(rs.first) < best_time)))
                    {
                        best_id = rs.first;
                        best_time = player_.single_waypoint_.last_visited_.at(rs.first);
                    }
                }
                if (best_id == SIZE_MAX) {
                    THROW_OR_ABORT("Select next waypoint failed. Forgot diagonal elements of adjacency matrix?");
                }
                set_waypoint(best_id);
            }
        }
    }
}
