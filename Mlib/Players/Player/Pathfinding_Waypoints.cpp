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
#include <Mlib/Scene_Graph/Driving_Direction.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

PathfindingWaypoints::PathfindingWaypoints(Player& player)
: player_{player},
  waypoint_{ fixed_nans <float, 3>()},
  waypoint_id_{ SIZE_MAX },
  waypoint_reached_{ false },
  nwaypoints_reached_{ 0 },
  record_waypoints_{ false }
{}

void PathfindingWaypoints::set_waypoint(const FixedArray<float, 3>& waypoint, size_t waypoint_id) {
    waypoint_ = waypoint;
    waypoint_id_ = waypoint_id;
    if (record_waypoints_ && !any(Mlib::isnan(waypoint))) {
        waypoint_history_.push_back(waypoint);
    }
    waypoint_reached_ = false;
}

void PathfindingWaypoints::set_waypoint(const FixedArray<float, 3>& waypoint) {
    set_waypoint(waypoint, SIZE_MAX);
}

void PathfindingWaypoints::set_waypoint(size_t waypoint_id) {
    set_waypoint(waypoints().points.at(waypoint_id), waypoint_id);
}

void PathfindingWaypoints::set_waypoints(
    const SceneNode& node,
    const std::map<WayPointLocation, PointsAndAdjacency<float, 3>>& all_waypoints)
{
    all_waypoints_bvh_.clear();
    all_waypoints_ = all_waypoints;
    TransformationMatrix<float, 3> m = node.absolute_model_matrix();
    for (auto& wps : all_waypoints_) {
        wps.second.adjacency = wps.second.adjacency * m.get_scale();
        Bvh<float, size_t, 3> bvh{{100.f, 100.f, 100.f}, 10};
        size_t i = 0;
        for (FixedArray<float, 3>& p : wps.second.points) {
            p = m.transform(p);
            bvh.insert(p, i++);
        }
        // bvh.optimize_search_time(std::cout);
        if (!all_waypoints_bvh_.insert({wps.first, std::move(bvh)}).second) {
            throw std::runtime_error("Could not insert waypoints");
        }
    }
    last_visited_ = std::vector<std::chrono::time_point<std::chrono::steady_clock>>(waypoints().points.size());
    waypoint_id_ = SIZE_MAX;
    nwaypoints_reached_ = 0;
}

void PathfindingWaypoints::draw_waypoint_history(const std::string& filename) const {
    player_.delete_node_mutex_.notify_reading();
    if (!record_waypoints_) {
        throw std::runtime_error("draw_waypoint_history but recording is not enabled");
    }
    std::ofstream ofstr{filename};
    if (ofstr.fail()) {
        throw std::runtime_error("Could not open \"" + filename + "\" for write");
    }
    Svg<float> svg{ofstr, 600, 600};
    std::vector<float> x;
    std::vector<float> y;
    x.resize(waypoint_history_.size());
    y.resize(waypoint_history_.size());
    size_t i = 0;
    for (const auto& w : waypoint_history_) {
        x[i] = w(0);
        y[i] = w(2);
        ++i;
    }
    svg.plot(x, y);
    svg.finish();
    ofstr.flush();
    if (ofstr.fail()) {
        throw std::runtime_error("Could not write to file \"" + filename + '"');
    }
}

const PointsAndAdjacency<float, 3>& PathfindingWaypoints::waypoints() const {
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
    FixedArray<float, 3> pos3 = player_.vehicle_.rb->rbi_.abs_position();
    if (waypoint_id_ == SIZE_MAX) {
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
                (dot0d(diff / std::sqrt(dist2), z3) < -std::cos(45.f * degrees)))
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
        if (waypoint_reached_) {
            if (nwaypoints_reached_ < 2) {
                // If we already have less than two waypoints, go further forward.
                size_t best_id = SIZE_MAX;
                float best_distance = INFINITY;
                for (const auto& rs : waypoints().adjacency.column(waypoint_id_)) {
                    float dist = dot0d(waypoints().points.at(rs.first) - pos3, z3);
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
                for (const auto& rs : waypoints().adjacency.column(waypoint_id_)) {
                    if ((best_id == SIZE_MAX) ||
                        (last_visited_.at(rs.first) == deflt) ||
                        ((best_time != deflt) && (last_visited_.at(rs.first) < best_time)))
                    {
                        best_id = rs.first;
                        best_time = last_visited_.at(rs.first);
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

void PathfindingWaypoints::move_to_waypoint() {
    player_.delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!player_.skills_.can_drive) {
        return;
    }
    if (!player_.has_rigid_body()) {
        return;
    }
    player_.vehicle_.rb->vehicle_controller().reset(
        0.f, // surface_power
        0.f  // steer_angle
    );
    if (std::isnan(player_.surface_power_forward_) ||
        std::isnan(player_.surface_power_backward_) ||
        any(Mlib::isnan(waypoint_)))
    {
        player_.step_on_brakes();
        player_.vehicle_.rb->vehicle_controller().apply();
        return;
    }
    // Stop when distance to waypoint is small enough (break).
    if (!player_.ramming()) {
        FixedArray<float, 3> pos3 = player_.vehicle_.rb->rbi_.abs_position();
        if (sum(squared(pos3 - waypoint_)) < squared(player_.driving_mode_.rest_radius)) {
            player_.step_on_brakes();
            player_.vehicle_.rb->vehicle_controller().apply();
            if (waypoint_id_ != SIZE_MAX) {
                last_visited_.at(waypoint_id_) = std::chrono::steady_clock::now();
            }
            waypoint_reached_ = true;
            ++nwaypoints_reached_;
            return;
        }
    }
    float d_wpt = 0;
    // Avoid collisions with other players (break).
    for (const auto& [_, p] : player_.players_.players()) {
        if (p.get() == &player_) {
            continue;
        }
        if (!p->has_rigid_body()) {
            continue;
        }
        if (player_.ramming() &&
            (p->vehicle_.rb == player_.target_rb_))
        {
            continue;
        }
        FixedArray<float, 3> d = p->vehicle_.rb->rbi_.abs_position() - player_.vehicle_.rb->rbi_.abs_position();
        float dl2 = sum(squared(d));
        if (dl2 < squared(player_.driving_mode_.collision_avoidance_radius_break)) {
            auto z = player_.vehicle_.rb->rbi_.abs_z();
            if (dot0d(d, z) < 0) {
                player_.step_on_brakes();
                player_.vehicle_.rb->vehicle_controller().apply();
                return;
            }
        } else if (dl2 < squared(player_.driving_mode_.collision_avoidance_radius_correct)) {
            if (dl2 > 1e-12) {
                auto z = player_.vehicle_.rb->rbi_.abs_z();
                if (dot0d(d, z) / std::sqrt(dl2) < -player_.driving_mode_.collision_avoidance_cos) {
                    if (player_.driving_direction_ == DrivingDirection::CENTER || player_.driving_direction_ == DrivingDirection::RIGHT) {
                        d_wpt = player_.driving_mode_.collision_avoidance_delta;
                    } else if (player_.driving_direction_ == DrivingDirection::LEFT) {
                        d_wpt = -player_.driving_mode_.collision_avoidance_delta;
                    } else {
                        throw std::runtime_error("Unknown driving direction");
                    }
                }
            }
        }
    }
    // Keep velocity within the specified range.
    {
        float dvel = -dot0d(player_.vehicle_.rb->rbi_.rbp_.v_, player_.vehicle_.rb->rbi_.abs_z()) - player_.driving_mode_.max_velocity;
        if (dvel < 0) {
            player_.drive_forward();
        } else if (dvel < player_.driving_mode_.max_delta_velocity_break) {
            player_.roll_tires();
        } else {
            player_.step_on_brakes();
        }
    }
    // Steer towards waypoint.
    // FixedArray<float, 3> wp{waypoint_(0), 0, waypoint_(1)};
    // auto m = vehicle_.rb->get_new_absolute_model_matrix();
    // auto v = inverted_scaled_se3(m);
    // auto wpt = dehomogenized_3(dot1d(v, homogenized_4(wp)));
    auto z3 = player_.vehicle_.rb->rbi_.rbp_.abs_z();
    FixedArray<float, 2> z{z3(0), z3(2)};
    float zl2 = sum(squared(z));
    if (zl2 > 1e-12) {
        z /= std::sqrt(zl2);
        auto p = player_.vehicle_.rb->rbi_.rbp_.abs_position();
        auto wpt = FixedArray<float, 2>{waypoint_(0), waypoint_(2)} - FixedArray<float, 2>{p(0), p(2)};
        FixedArray<float, 2, 2> m{
            z(1), -z(0),
            z(0), z(1)};
        wpt = dot1d(m, wpt);
        if (sum(squared(wpt)) > 1e-12) {
            wpt += FixedArray<float, 2>(-wpt(1), wpt(0)) * d_wpt;
            if (wpt(1) > 0) {
                // The waypoint is behind us => full, inverted steering.
                if (wpt(0) < 0) {
                    player_.steer_left_full();
                } else {
                    player_.steer_right_full();
                }
            } else {
                // The waypoint is in front of us => partial, inverted steering.
                float angle = std::atan(std::abs(wpt(0) / wpt(1)));
                if (wpt(0) < 0) {
                    player_.steer_left_partial(angle);
                } else {
                    player_.steer_right_partial(angle);
                }
            }
        }
    }
    player_.vehicle_.rb->vehicle_controller().apply();
}

void PathfindingWaypoints::notify_spawn() {
    player_.delete_node_mutex_.assert_this_thread_is_deleter_thread();
    for (auto& l : last_visited_) {
        l = std::chrono::time_point<std::chrono::steady_clock>();
    }
    set_waypoint(fixed_nans<float, 3>());
    nwaypoints_reached_ = 0;
    waypoint_history_.clear();
}
