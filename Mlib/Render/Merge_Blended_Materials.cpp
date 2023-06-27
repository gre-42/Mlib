#include "Merge_Blended_Materials.hpp"
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Uv_Tile.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <set>

using namespace Mlib;

void Mlib::merge_blended_materials(
    const std::string& mesh_resource_name,
    const std::string& merged_texture_name,
    const std::string& merged_array_name,
    SceneNodeResources& scene_node_resources,
    RenderingResources& rendering_resources)
{
    auto mesh = scene_node_resources.get_animated_arrays(mesh_resource_name);
    std::set<std::string> filenames;
    for (const auto& cva : mesh->dcvas) {
        if (cva->material.blend_mode == BlendMode::CONTINUOUS) {
            if (cva->material.textures.size() != 1) {
                THROW_OR_ABORT("Material \"" + cva->material.identifier() + "\" does not have exactly one texture");
            }
            // if (cva->material.textures[0].texture_descriptor.color.find("tree") == std::string::npos) {
            //     continue;
            // }
            filenames.insert(cva->material.textures[0].texture_descriptor.color);
        }
    }
    auto uv_tiles = rendering_resources.generate_texture_atlas(merged_texture_name, filenames);
    scene_node_resources.merge_materials(
        mesh_resource_name,
        merged_array_name,
        Material{
            .blend_mode = BlendMode::CONTINUOUS,
            .textures = {{.texture_descriptor = {.color = merged_texture_name, .color_mode = ColorMode::RGBA}}},
            .aggregate_mode = AggregateMode::ONCE,
            .emissivity = OrderableFixedArray<float, 3>{0.f, 0.f, 0.f},
            .ambience = OrderableFixedArray<float, 3>{1.f, 1.f, 1.f},
            .diffusivity = OrderableFixedArray<float, 3>{0.f, 0.f, 0.f},
            .specularity = OrderableFixedArray<float, 3>{0.f, 0.f, 0.f}},
        uv_tiles);
}
