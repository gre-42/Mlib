#pragma once
#include <Mlib/Players/Player/Supply_Depots_Waypoints.hpp>
#include <unordered_map>

namespace Mlib {

class Navigate;
enum class JoinedWayPointSandbox;
class SupplyDepots;

class SupplyDepotsWaypointsCollection {
public:
    explicit SupplyDepotsWaypointsCollection(
        const SupplyDepots& supply_depots,
        const Navigate& navigate);
    const SupplyDepotsWaypoints& get_way_points(JoinedWayPointSandbox key) const;
private:
    mutable std::unordered_map<JoinedWayPointSandbox, SupplyDepotsWaypoints> sdw_;
    const SupplyDepots& supply_depots_;
    const Navigate& navigate_;
};

}
