#pragma once
#include <map>
#include <string>

namespace Mlib {

class ResourceNameCycle;
class BatchResourceInstantiator;
class StreetBvh2d;
class GroundBvh;
struct Node;

void add_trees_to_tree_nodes(
    BatchResourceInstantiator& bri,
    // std::list<SteinerPointInfo>& steiner_points,
    ResourceNameCycle& rnc,
    float min_dist_to_road,
    const StreetBvh2d& street_bvh,
    const GroundBvh& ground_bvh,
    const std::map<std::string, Node>& nodes,
    float scale);

}
