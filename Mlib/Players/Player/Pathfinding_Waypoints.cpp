#include "Pathfinding_Waypoints.hpp"
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Mesh/Points_And_Adjacency.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Rigid_Body_Vehicle_Controller.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

PathfindingWaypoints::PathfindingWaypoints(Player& player)
: player_{player}
{}

PathfindingWaypoints::~PathfindingWaypoints()
{}

void PathfindingWaypoints::set_waypoint(size_t waypoint_id) {
    player_.single_waypoint_.set_waypoint(waypoints().points.at(waypoint_id), waypoint_id);
}

void PathfindingWaypoints::set_waypoints(
    const SceneNode& node,
    const std::map<WayPointLocation, PointsAndAdjacency<double, 3>>& all_waypoints)
{
    all_waypoints_bvh_.clear();
    all_waypoints_ = all_waypoints;
    TransformationMatrix<float, double, 3> m = node.absolute_model_matrix();
    for (auto& wps : all_waypoints_) {
        wps.second.adjacency = wps.second.adjacency * (double)m.get_scale();
        Bvh<double, size_t, 3> bvh{{100.f, 100.f, 100.f}, 10};
        size_t i = 0;
        for (FixedArray<double, 3>& p : wps.second.points) {
            p = m.transform(p);
            bvh.insert(p, i++);
        }
        // bvh.optimize_search_time(std::cout);
        if (!all_waypoints_bvh_.insert({wps.first, std::move(bvh)}).second) {
            throw std::runtime_error("Could not insert waypoints");
        }
    }
    player_.single_waypoint_.notify_set_waypoints(waypoints().points.size());
}

const PointsAndAdjacency<double, 3>& PathfindingWaypoints::waypoints() const {
    player_.delete_node_mutex_.notify_reading();
    auto it = all_waypoints_.find(player_.driving_mode_.way_point_location);
    if (it == all_waypoints_.end()) {
        throw std::runtime_error("Could not find waypoints for the specified location");
    }
    return it->second;
}

bool PathfindingWaypoints::has_waypoints() const {
    player_.delete_node_mutex_.notify_reading();
    auto it = all_waypoints_.find(player_.driving_mode_.way_point_location);
    if (it == all_waypoints_.end()) {
        return false;
    }
    return !it->second.points.empty();
}

void PathfindingWaypoints::select_next_waypoint() {
    player_.delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!waypoints().adjacency.initialized()) {
        return;
    }
    if (!player_.has_rigid_body()) {
        return;
    }
    FixedArray<float, 3> z3 = player_.vehicle_.rb->rbi_.abs_z();
    FixedArray<double, 3> pos3 = player_.vehicle_.rb->rbi_.abs_position();
    if (player_.single_waypoint_.waypoint_id_ == SIZE_MAX) {
        // If we have no current waypoint, find closest point in waypoints array.
        float max_distance = 100;
        size_t closest_id = SIZE_MAX;
        float closest_distance2 = INFINITY;
        const auto& wps = waypoints();
        all_waypoints_bvh_.at(player_.driving_mode_.way_point_location).visit(
            {pos3, max_distance},
            [&](size_t i)
        {
            const auto& rs = wps.points.at(i);
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
                float best_distance = INFINITY;
                for (const auto& rs : waypoints().adjacency.column(player_.single_waypoint_.waypoint_id_)) {
                    float dist = dot0d(waypoints().points.at(rs.first) - pos3, z3.casted<double>());
                    if (dist < best_distance) {
                        best_id = rs.first;
                        best_distance = dist;
                    }
                }
                if (best_id == SIZE_MAX) {
                    throw std::runtime_error("Select next waypoint failed. Forgot diagonal elements of adjacency matrix?");
                }
                set_waypoint(best_id);
            } else {
                // If we already have two waypoints, pick oldest neighbor.
                auto deflt = std::chrono::time_point<std::chrono::steady_clock>();
                size_t best_id = SIZE_MAX;
                auto best_time = deflt;
                for (const auto& rs : waypoints().adjacency.column(player_.single_waypoint_.waypoint_id_)) {
                    if ((best_id == SIZE_MAX) ||
                        (player_.single_waypoint_.last_visited_.at(rs.first) == deflt) ||
                        ((best_time != deflt) && (player_.single_waypoint_.last_visited_.at(rs.first) < best_time)))
                    {
                        best_id = rs.first;
                        best_time = player_.single_waypoint_.last_visited_.at(rs.first);
                    }
                }
                if (best_id == SIZE_MAX) {
                    throw std::runtime_error("Select next waypoint failed. Forgot diagonal elements of adjacency matrix?");
                }
                set_waypoint(best_id);
            }
        }
    }
}
