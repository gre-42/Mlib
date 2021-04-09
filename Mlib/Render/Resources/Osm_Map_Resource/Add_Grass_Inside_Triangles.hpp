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
class TriangleList;

void add_grass_inside_triangles(
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    std::map<std::string, std::list<FixedArray<float, 3>>>& hitboxes,
    ResourceNameCycle& rnc,
    const TriangleList& triangles,
    float scale,
    float distance);

}
