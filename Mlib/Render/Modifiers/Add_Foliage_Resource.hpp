#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <string>
#include <vector>

namespace Mlib {

class SceneNodeResources;
template <typename TData, size_t... tshape>
class FixedArray;
struct ParsedResourceName;
enum class UpAxis;

void add_foliage_resource(
    const std::string& mesh_resource_name,
    const std::string& foliage_resource_name,
    SceneNodeResources& scene_node_resources,
    const std::vector<ParsedResourceName>& near_grass_resources,
    const std::vector<ParsedResourceName>& dirty_near_grass_resources,
    ScenePos near_grass_distance,
    const std::string& near_grass_foliagemap,
    float near_grass_foliagemap_scale,
    float scale,
    UpAxis up_axis);

}
