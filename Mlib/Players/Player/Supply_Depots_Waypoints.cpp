#include "Supply_Depots_Waypoints.hpp"
#include <Mlib/Geometry/Shortest_Path_Multiple_Targets.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Game_Logic/Supply_Depots.hpp>
#include <Mlib/Players/Player/Single_Waypoint.hpp>

using namespace Mlib;

#pragma GCC push_options
#pragma GCC optimize("O0")
SupplyDepotsWaypoints::SupplyDepotsWaypoints(
    Player& player,
    SingleWaypoint& single_waypoint,
    SupplyDepots& supply_depots)
: player_{player},
  single_waypoint_{single_waypoint},
  supply_depots_{supply_depots}
{}

struct WaypointAndTTotalDistance {
    double ttotal_distance;
    size_t waypoint_id;
};

bool SupplyDepotsWaypoints::select_next_waypoint() {
    if (!player_.needs_supplies()) {
        return false;
    }
    if (single_waypoint_.previous_waypoint_id() == SIZE_MAX) {
        return false;
    }
    if (single_waypoint_.target_waypoint_id() == SIZE_MAX) {
        return false;
    }
    auto p = player_.rigid_body().rbi_.abs_position();
    auto compute_ttotal_distance = [this, &p](size_t waypoint_id) {
        return WaypointAndTTotalDistance{
            .ttotal_distance = std::sqrt(sum(squared(p - waypoint_positions_.at(waypoint_id)))) + total_distances_.at(waypoint_id),
            .waypoint_id = waypoint_id};
    };
    auto ctarget = compute_ttotal_distance(single_waypoint_.target_waypoint_id());
    auto cprev = compute_ttotal_distance(single_waypoint_.previous_waypoint_id());
    if (ctarget.ttotal_distance < cprev.ttotal_distance) {
        single_waypoint_.set_waypoint(waypoint_positions_.at(ctarget.waypoint_id), ctarget.waypoint_id);
        return true;
    } else if (cprev.ttotal_distance < ctarget.ttotal_distance) {
        single_waypoint_.set_waypoint(waypoint_positions_.at(cprev.waypoint_id), cprev.waypoint_id);
        return true;
    }
    return false;
}

void SupplyDepotsWaypoints::set_waypoints(const PointsAndAdjacency<double, 3>& waypoints)
{
    std::vector<size_t> targets;
    targets.reserve(waypoints.points.size());
    size_t i = 0;
    for (const auto& p : waypoints.points) {
        if (!supply_depots_.visit_supply_depots(
            p,
            [](const SupplyDebot& supply_depot)
            {
                return false;
            }))
        {
            targets.push_back(i);
        }
        ++i;
    }
    waypoint_positions_ = waypoints.points;
    shortest_path_multiple_targets(
        waypoints,
        targets,
        predecessors_,
        total_distances_);
}

#pragma GCC pop_options