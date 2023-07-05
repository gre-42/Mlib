#pragma once
#include <string>

namespace Mlib {

class SceneNodeResources;
struct ColoredVertexArrayFilter;
template <typename TData, size_t... tshape>
class FixedArray;

void add_foliage_resource(
    const std::string& mesh_resource_name,
    const std::string& foliage_resource_name,
    SceneNodeResources& scene_node_resources,
    const ColoredVertexArrayFilter& grass_filter,
    float scale,
    const FixedArray<float, 3>& up);

}
