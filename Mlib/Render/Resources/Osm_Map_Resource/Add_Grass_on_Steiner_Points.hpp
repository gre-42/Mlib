#pragma once
#include <list>
#include <map>
#include <string>

namespace Mlib {

struct ResourceInstanceDescriptor;
struct ObjectResourceDescriptor;
class ResourceNameCycle;
struct SteinerPointInfo;
template <typename TData, size_t... tshape>
class FixedArray;
class StreetBvh;

void add_grass_on_steiner_points(
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& hitboxes,
    ResourceNameCycle& rnc,
    const StreetBvh& ground_bvh,
    const StreetBvh& air_bvh,
    const std::list<SteinerPointInfo>& steiner_points,
    float scale,
    float dmin,
    float dmax);

}
