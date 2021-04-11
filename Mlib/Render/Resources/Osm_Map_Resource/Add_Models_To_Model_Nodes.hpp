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
class GroundBvh;
struct Node;
struct Way;
class SceneNodeResources;

void add_models_to_model_nodes(
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    std::map<std::string, std::list<FixedArray<float, 3>>>& hitboxes,
    const GroundBvh& ground_bvh,
    const SceneNodeResources& resources,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    float scale);

}
