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
struct Node;
struct Way;
class StreetBvh;

void add_trees_to_forest_outlines(
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    std::map<std::string, std::list<FixedArray<float, 3>>>& hitboxes,
    std::list<SteinerPointInfo>& steiner_points,
    ResourceNameCycle& rnc,
    float min_dist_to_road,
    const StreetBvh& ground_bvh,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    float tree_distance,
    float tree_inwards_distance,
    float scale);

}
