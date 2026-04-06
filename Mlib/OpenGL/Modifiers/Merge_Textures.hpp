#pragma once
#include <string>

namespace Mlib {

template <class T>
class VariableAndHash;
class SceneNodeResources;
class RenderingResources;
struct MergedTexturesConfig;

void merge_textures(
    const VariableAndHash<std::string>& mesh_resource_name,
    const MergedTexturesConfig& merged_materials_config,
    SceneNodeResources& scene_node_resources,
    RenderingResources& rendering_resources);

}
