#pragma once
#include <string>
#include <vector>

namespace Mlib {

template <class T>
class VariableAndHash;
class SceneNodeResources;
class RenderingResources;
enum class UpAxis;

void replace_terrain_material(
    const VariableAndHash<std::string>& resource_name,
    const std::vector<VariableAndHash<std::string>>& textures,
    double scale,
    double uv_scale,
    double uv_period,
    UpAxis up_axis,
    SceneNodeResources& scene_node_resources,
    RenderingResources& rendering_resources);

}
