#pragma once
#include <string>

namespace Mlib {

class SceneNodeResources;
class RenderingResources;
struct MergedTexturesConfig;

void merge_textures(
    const std::string& mesh_resource_name,
    const MergedTexturesConfig& merged_materials_config,
    SceneNodeResources& scene_node_resources,
    RenderingResources& rendering_resources);

}
