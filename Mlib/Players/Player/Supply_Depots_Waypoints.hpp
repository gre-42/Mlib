#pragma once
#include <cstddef>
#include <vector>

namespace Mlib {

class Player;
class SingleWaypoint;
class SupplyDepots;
template <class TData, size_t tndim>
struct PointsAndAdjacency;
template <typename TData, size_t... tshape>
class FixedArray;

class SupplyDepotsWaypoints {
public:
    SupplyDepotsWaypoints(
        Player& player,
        SingleWaypoint& single_waypoint,
        SupplyDepots& supply_depots);
    bool select_next_waypoint();
    void set_waypoints(const PointsAndAdjacency<double, 3>& waypoints);
private:
    Player& player_;
    SingleWaypoint& single_waypoint_;
    SupplyDepots& supply_depots_;
    std::vector<FixedArray<double, 3>> waypoint_positions_;
    std::vector<size_t> predecessors_;
    std::vector<double> total_distances_;
};

}
