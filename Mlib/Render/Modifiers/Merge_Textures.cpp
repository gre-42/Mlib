#include "Merge_Textures.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Material/Merged_Texture_Name.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Geometry/Mesh/Uv_Tile.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Render/Modifiers/Merged_Textures_Config.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <map>
#include <ranges>

using namespace Mlib;

void Mlib::merge_textures(
    const std::string& mesh_resource_name,
    const MergedTexturesConfig& merged_materials_config,
    SceneNodeResources& scene_node_resources,
    RenderingResources& rendering_resources)
{
    scene_node_resources.add_modifier(
        mesh_resource_name,
        [&scene_node_resources,
         &rendering_resources,
         mesh_resource_name,
         merged_materials_config]
        (ISceneNodeResource& scene_node_resource){
            std::map<std::string, std::list<ColoredVertexArray<double>*>> merged_filenames;
            auto meshes = scene_node_resource.get_rendering_arrays();
            for (const auto& mesh : meshes) {
                for (const auto& cva : mesh->dcvas) {
                    if (!cva->material.merge_textures) {
                        goto skip;
                    }
                    if (!any(cva->material.blend_mode & BlendMode::ANY_CONTINUOUS)) {
                        lwarn() << "Attempt to merge object \"" << cva->name << "\", which has a non-continuous material";
                        goto skip;
                    }
                    for (auto& t : cva->triangles) {
                        auto min_uv_floor =
                            minimum(minimum(t(0).uv, t(1).uv), t(2).uv)
                            .applied([](float v){return std::floor(v);});
                        for (auto& v : t.flat_iterable()) {
                            v.uv -= min_uv_floor;
                        }
                        for (const auto& v : t.flat_iterable()) {
                            if (any(v.uv < 0.f) || any(v.uv > 1.f)) {
                                lwarn() << "UV-coordinates of object \"" << cva->name << "\" do no permit texture atlas";
                                goto skip;
                            }
                        }
                    }
                    merged_filenames[MergedTextureName{cva->material}.name].push_back(cva.get());
                    continue;
                    skip:
                    if (any(cva->material.blend_mode & BlendMode::ANY_CONTINUOUS)) {
                        cva->material.blend_mode = BlendMode::BINARY_05;
                    }
                }
            }
            auto keys = [](const auto& container){
                std::vector<std::string> result;
                result.reserve(container.size());
                for (const auto& [k, _] : container) {
                    result.push_back(k);
                }
                return result;
            };
            // auto keys = std::views::keys(merged_filenames);
            // auto uv_tiles = rendering_resources.generate_texture_atlas(merged_texture_name, std::set(keys.begin(), keys.end()));
            auto uv_tiles = rendering_resources.generate_auto_texture_atlas(merged_materials_config.texture_name, keys(merged_filenames));
            // rendering_resources.save_to_file("/tmp/atlas.png", TextureDescriptor{.color = merged_texture_name, .color_mode = ColorMode::RGBA});
            
            std::list<FixedArray<ColoredVertex<double>, 3>> merged_triangles;
            std::list<FixedArray<uint8_t, 3>> merged_triangle_texture_layers;
            for (const auto& [filename, cvas] : merged_filenames) {
                const auto& tile = uv_tiles.at(filename);
                for (const auto& cva : cvas) {
                    for (const auto& tri : cva->triangles) {
                        {
                            auto& mtri = merged_triangles.emplace_back(tri);
                            for (auto& v : mtri.flat_iterable()) {
                                assert_true(all(v.uv >= 0.f));
                                assert_true(all(v.uv <= 1.f));
                                v.uv = tile.position + v.uv * tile.size;
                            }
                        }
                        {
                            auto& mlay = merged_triangle_texture_layers.emplace_back();
                            for (auto& v : mlay.flat_iterable()) {
                                v = tile.layer;
                            }
                        }
                    }
                    cva->physics_material &= ~PhysicsMaterial::ATTR_VISIBLE;
                }
            }
            scene_node_resources.add_resource(
                merged_materials_config.resource_name,
                std::make_shared<ColoredVertexArrayResource>(
                    std::make_shared<ColoredVertexArray<double>>(
                    merged_materials_config.array_name,
                    Material{
                        .blend_mode = merged_materials_config.blend_mode,
                        .textures = {{.texture_descriptor = {
                            .color = merged_materials_config.texture_name,
                            .color_mode = ColorMode::RGBA,
                            .mipmap_mode = MipmapMode::WITH_MIPMAPS}}},
                        .aggregate_mode = merged_materials_config.aggregate_mode,
                        .max_triangle_distance = merged_materials_config.max_triangle_distance,
                        .cull_faces = merged_materials_config.cull_faces,
                        .emissivity = OrderableFixedArray<float, 3>{0.f, 0.f, 0.f},
                        .ambience = OrderableFixedArray<float, 3>{1.f, 1.f, 1.f},
                        .diffusivity = OrderableFixedArray<float, 3>{0.f, 0.f, 0.f},
                        .specularity = OrderableFixedArray<float, 3>{0.f, 0.f, 0.f}},
                    PhysicsMaterial::ATTR_VISIBLE,
                    std::vector<FixedArray<ColoredVertex<double>, 3>>(merged_triangles.begin(), merged_triangles.end()),
                    std::vector<FixedArray<ColoredVertex<double>, 2>>{},
                    std::vector<FixedArray<std::vector<BoneWeight>, 3>>{},
                    std::vector<FixedArray<std::vector<BoneWeight>, 2>>{},
                    std::vector<FixedArray<uint8_t, 3>>(merged_triangle_texture_layers.begin(), merged_triangle_texture_layers.end()),
                    std::vector<FixedArray<uint8_t, 2>>{})));
            scene_node_resources.add_companion(
                mesh_resource_name,
                merged_materials_config.resource_name,
                RenderableResourceFilter{});
        });
}
