#pragma once
#include <string>
#include <vector>

namespace Mlib {

class SceneNodeResources;
class RenderingResources;
struct MergedTextureFilter;
enum class UpAxis;

void replace_terrain_material(
    const std::string& resource_name,
    const std::vector<std::string>& textures,
    double scale,
    double uv_scale,
    double uv_period,
    UpAxis up_axis,
    SceneNodeResources& scene_node_resources,
    RenderingResources& rendering_resources,
    const MergedTextureFilter& filter);

}
