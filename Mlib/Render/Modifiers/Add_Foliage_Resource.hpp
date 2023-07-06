#pragma once
#include <string>
#include <vector>

namespace Mlib {

class SceneNodeResources;
struct ColoredVertexArrayFilter;
template <typename TData, size_t... tshape>
class FixedArray;
struct ParsedResourceName;

void add_foliage_resource(
    const std::string& mesh_resource_name,
    const std::string& foliage_resource_name,
    SceneNodeResources& scene_node_resources,
    const ColoredVertexArrayFilter& grass_filter,
    const std::vector<ParsedResourceName>& near_grass_resources,
    const std::vector<ParsedResourceName>& dirty_near_grass_resources,
    double near_grass_distance,
    const std::string& near_grass_foliagemap,
    float near_grass_foliagemap_scale,
    float scale,
    const FixedArray<float, 3>& up);

}
