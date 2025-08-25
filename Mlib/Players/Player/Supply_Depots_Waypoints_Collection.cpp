#include "Supply_Depots_Waypoints_Collection.hpp"
#include <Mlib/Players/Game_Logic/Navigate.hpp>
#include <Mlib/Scene_Graph/Interfaces/Way_Points.hpp>

using namespace Mlib;

SupplyDepotsWaypointsCollection::SupplyDepotsWaypointsCollection(
    const SupplyDepots& supply_depots,
    const Navigate& navigate)
    : supply_depots_{ supply_depots }
    , navigate_{ navigate }
{}

const SupplyDepotsWaypoints& SupplyDepotsWaypointsCollection::get_way_points(JoinedWayPointSandbox key) const
{
    auto it = sdw_.find(key);
    if (it == sdw_.end()) {
        const auto& wpts = navigate_.way_points(key);
        if (!sdw_.try_emplace(key, wpts.way_points, supply_depots_).second) {
            verbose_abort("SupplyDepotsWaypointsCollection::get_way_points data race");
        }
    }
    return it->second;
}
