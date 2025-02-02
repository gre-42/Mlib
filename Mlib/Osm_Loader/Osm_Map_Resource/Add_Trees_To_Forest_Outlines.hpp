#pragma once
#include <map>
#include <string>

namespace Mlib {

class ResourceNameCycle;
class BatchResourceInstantiator;
struct Node;
struct Way;
class StreetBvh;
class GroundBvh;

void add_trees_to_forest_outlines(
    BatchResourceInstantiator& bri,
    // std::list<SteinerPointInfo>& steiner_points,
    ResourceNameCycle& rnc,
    double min_dist_to_road,
    const StreetBvh& street_bvh,
    const GroundBvh& ground_bvh,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    double tree_distance,
    double tree_inwards_distance,
    double scale);

}
