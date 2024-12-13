#include "Supply_Depots_Waypoints.hpp"
#include <Mlib/Geometry/Shortest_Path_Multiple_Targets.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Game_Logic/Supply_Depots.hpp>
#include <Mlib/Players/Player/Single_Waypoint.hpp>

using namespace Mlib;

SupplyDepotsWaypoints::SupplyDepotsWaypoints(
    Player& player,
    SingleWaypoint& single_waypoint,
    SupplyDepots& supply_depots)
    : player_{ player }
    , single_waypoint_{ single_waypoint }
    , supply_depots_{ supply_depots }
{}

struct WaypointAndTTotalDistance {
    CompressedScenePos ttotal_distance;
    size_t waypoint_id;
};

bool SupplyDepotsWaypoints::select_next_waypoint() {
    // if (player_.name() == "npc1") {
    //     for (size_t i = 0; i < waypoint_positions_.size(); ++i) {
    //         if ((total_distances_.at(i) == INFINITY) || (total_distances_.at(i) == 0)) {
    //             continue;
    //         }
    //         g_beacons.push_back(
    //             Beacon{
    //                 .location = TransformationMatrix<float, ScenePos, 3>{
    //                     (float)total_distances_.at(i) / (100.f * meters) * fixed_identity_array<float, 3>(),
    //                     waypoint_positions_.at(i)
    //                 },
    //                 .resource_name = "box_on_ground"
    //             });
    //     }
    // }
    if (!player_.has_gun_node()) {
        return false;
    }
    if (!player_.needs_supplies()) {
        return false;
    }
    if (single_waypoint_.previous_waypoint_id() == SIZE_MAX) {
        return false;
    }
    if (single_waypoint_.target_waypoint_id() == SIZE_MAX) {
        return false;
    }
    if (player_.single_waypoint().waypoint_reached()) {
        size_t predecessor_id = predecessors_.at(player_.single_waypoint().target_waypoint_id());
        if (predecessor_id == SIZE_MAX) {
            return false;
        }
        player_.single_waypoint().set_waypoint(waypoint_positions_.at(predecessor_id), predecessor_id);
        return true;
    } else {
        auto p = player_.rigid_body().rbp_.abs_position().casted<CompressedScenePos>();
        auto compute_ttotal_distance = [this, &p](size_t waypoint_id) {
            // g_beacons.push_back(
            //     Beacon{
            //         .location = TransformationMatrix<float, ScenePos, 3>{
            //             0.2f * fixed_identity_array<float, 3>(),
            //             waypoint_positions_.at(waypoint_id)
            //         },
            //         .resource_name = "flag"
            //     });
            return WaypointAndTTotalDistance{
                .ttotal_distance = (CompressedScenePos)std::sqrt(sum(squared(p - waypoint_positions_.at(waypoint_id).position))) + total_distances_.at(waypoint_id),
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
}

void SupplyDepotsWaypoints::set_waypoints(const PointsAndAdjacencyResource& waypoints)
{
    std::vector<size_t> targets;
    targets.reserve(waypoints.points.size());
    size_t i = 0;
    for (const auto& p : waypoints.points) {
        if (!supply_depots_.visit_supply_depots(
            p.position,
            [](const SupplyDepot& supply_depot)
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
