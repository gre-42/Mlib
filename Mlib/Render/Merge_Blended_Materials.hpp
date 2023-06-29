#pragma once
#include <string>

namespace Mlib {

class SceneNodeResources;
class RenderingResources;
struct MergedTextureFilter;

void merge_blended_materials(
    const std::string& mesh_resource_name,
    const std::string& merged_texture_name,
    const std::string& merged_array_name,
    SceneNodeResources& scene_node_resources,
    RenderingResources& rendering_resources,
    const MergedTextureFilter& filter);

}
