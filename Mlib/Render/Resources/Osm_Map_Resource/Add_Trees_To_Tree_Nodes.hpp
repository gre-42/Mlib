#pragma once
#include <list>
#include <map>
#include <string>

namespace Mlib {

class ResourceNameCycle;
struct ResourceInstanceDescriptor;
struct ObjectResourceDescriptor;
template <class TData, size_t... tshape>
class FixedArray;
struct SteinerPointInfo;
class StreetBvh;
struct Node;

void add_trees_to_tree_nodes(
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& hitboxes,
    std::list<SteinerPointInfo>& steiner_points,
    ResourceNameCycle& rnc,
    float min_dist_to_road,
    const StreetBvh& ground_bvh,
    const std::map<std::string, Node>& nodes,
    float scale);

}
