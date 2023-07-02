#pragma once
#include <string>

namespace Mlib {

class SceneNodeResources;
class RenderingResources;
struct MergedTextureFilter;
enum class BlendMode;

void merge_blended_materials(
    const std::string& mesh_resource_name,
    const std::string& merged_resource_name,
    const std::string& merged_texture_name,
    const std::string& merged_array_name,
    BlendMode merged_blend_mode,
    SceneNodeResources& scene_node_resources,
    RenderingResources& rendering_resources,
    const MergedTextureFilter& filter);

}
