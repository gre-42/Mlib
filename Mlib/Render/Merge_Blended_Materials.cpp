#include "Merge_Blended_Materials.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Geometry/Mesh/Uv_Tile.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <set>

using namespace Mlib;

void Mlib::merge_blended_materials(
    const std::string& mesh_resource_name,
    const std::string& merged_texture_name,
    const std::string& merged_array_name,
    SceneNodeResources& scene_node_resources,
    RenderingResources& rendering_resources,
    const ColoredVertexArrayFilter& filter)
{
    auto mesh = scene_node_resources.get_animated_arrays(mesh_resource_name);
    std::set<std::string> filenames;
    std::set<std::string> excluded_filenames;
    for (const auto& cva : mesh->dcvas) {
        if (cva->material.blend_mode == BlendMode::CONTINUOUS) {
            if (cva->material.textures.size() != 1) {
                THROW_OR_ABORT("Material \"" + cva->material.identifier() + "\" does not have exactly one texture");
            }
            if (!filter.matches(*cva)) {
                continue;
            }
            auto filename = cva->material.textures[0].texture_descriptor.color;
            if (excluded_filenames.contains(filename)) {
                continue;
            }
            for (auto& t : cva->triangles) {
                auto min_uv = minimum(minimum(t(0).uv, t(1).uv), t(2).uv);
                for (auto& v : t.flat_iterable()) {
                    for (size_t i = 0; i < 3; ++i) {
                        v.uv(i) -= std::floor(min_uv(i));
                    }
                }
                for (const auto& v : t.flat_iterable()) {
                    if (any(v.uv < 0.f) || any(v.uv > 1.f)) {
                        // if (filenames.contains(filename)) {
                        //     THROW_OR_ABORT("Filename \"" + filename + "\" already added");
                        // }
                        filenames.erase(filename);
                        excluded_filenames.insert(filename);
                        goto skip;
                    }
                }
            }
            filenames.insert(filename);
        }
        skip:;
    }
    for (const auto& cva : mesh->dcvas) {
        if (cva->material.blend_mode == BlendMode::CONTINUOUS) {
            auto filename = cva->material.textures[0].texture_descriptor.color;
            if (!filenames.contains(filename)) {
                cva->material.blend_mode = BlendMode::BINARY_05;
            }
        }
    }
    auto uv_tiles = rendering_resources.generate_texture_atlas(merged_texture_name, filenames);
    // rendering_resources.save_to_file("/tmp/atlas.png", TextureDescriptor{.color = merged_texture_name, .color_mode = ColorMode::RGBA});
    scene_node_resources.merge_materials(
        mesh_resource_name,
        merged_array_name,
        Material{
            .blend_mode = BlendMode::CONTINUOUS,
            .textures = {{.texture_descriptor = {
                .color = merged_texture_name,
                .color_mode = ColorMode::RGBA,
                .mipmap_mode = MipmapMode::WITH_MIPMAPS}}},
            .aggregate_mode = AggregateMode::ONCE,
            .cull_faces = false,
            .emissivity = OrderableFixedArray<float, 3>{0.f, 0.f, 0.f},
            .ambience = OrderableFixedArray<float, 3>{1.f, 1.f, 1.f},
            .diffusivity = OrderableFixedArray<float, 3>{0.f, 0.f, 0.f},
            .specularity = OrderableFixedArray<float, 3>{0.f, 0.f, 0.f}},
        PhysicsMaterial::ATTR_VISIBLE,
        uv_tiles);
}
