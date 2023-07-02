#include "Merge_Blended_Materials.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Material/Merged_Texture_Filter.hpp>
#include <Mlib/Geometry/Material/Merged_Texture_Name.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Uv_Tile.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <map>
#include <ranges>

using namespace Mlib;

void Mlib::merge_blended_materials(
    const std::string& mesh_resource_name,
    const std::string& merged_resource_name,
    const std::string& merged_texture_name,
    const std::string& merged_array_name,
    BlendMode merged_blend_mode,
    AggregateMode merged_aggregate_mode,
    SceneNodeResources& scene_node_resources,
    RenderingResources& rendering_resources,
    const MergedTextureFilter& filter)
{
    scene_node_resources.add_modifier(
        mesh_resource_name,
        [filter,
         &scene_node_resources,
         &rendering_resources,
         mesh_resource_name,
         merged_resource_name,
         merged_texture_name,
         merged_array_name,
         merged_blend_mode,
         merged_aggregate_mode]
        (ISceneNodeResource& scene_node_resource){
            std::map<std::string, std::list<ColoredVertexArray<double>*>> merged_filenames;
            std::map<std::string, std::list<ColoredVertexArray<double>*>> excluded_filenames;
            auto meshes = scene_node_resource.get_rendering_arrays();
            for (const auto& mesh : meshes) {
                for (const auto& cva : mesh->dcvas) {
                    MergedTextureName merged_texture_name{cva->material};
                    if (any(cva->material.blend_mode & BlendMode::ANY_CONTINUOUS)) {
                        if (cva->material.textures.size() != 1) {
                            THROW_OR_ABORT("Material \"" + cva->material.identifier() + "\" does not have exactly one texture");
                        }
                        if (!filter.matches(merged_texture_name)) {
                            goto skip;
                        }
                        if (excluded_filenames.contains(merged_texture_name.name)) {
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
                                    goto skip;
                                }
                            }
                        }
                        merged_filenames[merged_texture_name.name].push_back(cva.get());
                        continue;
                    }
                    skip:
                    excluded_filenames.insert(merged_filenames.extract(merged_texture_name.name));
                    excluded_filenames[merged_texture_name.name].push_back(cva.get());
                }
            }
            for (const auto& [_, cvas] : excluded_filenames) {
                for (const auto& cva : cvas) {
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
            auto uv_tiles = rendering_resources.generate_texture_atlas(merged_texture_name, keys(merged_filenames));
            // rendering_resources.save_to_file("/tmp/atlas.png", TextureDescriptor{.color = merged_texture_name, .color_mode = ColorMode::RGBA});
            
            std::list<FixedArray<ColoredVertex<double>, 3>> merged_tris;
            for (const auto& [filename, cvas] : merged_filenames) {
                const auto& tile = uv_tiles.at(filename);
                for (const auto& cva : cvas) {
                    for (const auto& tri : cva->triangles) {
                        auto& mtri = merged_tris.emplace_back(tri);
                        for (auto& v : mtri.flat_iterable()) {
                            assert_true(all(v.uv >= 0.f));
                            assert_true(all(v.uv <= 1.f));
                            v.uv = tile.position + v.uv * tile.size;
                        }
                    }
                    cva->physics_material &= ~PhysicsMaterial::ATTR_VISIBLE;
                }
            }
            scene_node_resources.add_resource(
                merged_resource_name,
                std::make_shared<ColoredVertexArrayResource>(
                    std::make_shared<ColoredVertexArray<double>>(
                    merged_array_name,
                    Material{
                        .blend_mode = merged_blend_mode,
                        .textures = {{.texture_descriptor = {
                            .color = merged_texture_name,
                            .color_mode = ColorMode::RGBA,
                            .mipmap_mode = MipmapMode::WITH_MIPMAPS}}},
                        .aggregate_mode = merged_aggregate_mode,
                        .cull_faces = false,
                        .emissivity = OrderableFixedArray<float, 3>{0.f, 0.f, 0.f},
                        .ambience = OrderableFixedArray<float, 3>{1.f, 1.f, 1.f},
                        .diffusivity = OrderableFixedArray<float, 3>{0.f, 0.f, 0.f},
                        .specularity = OrderableFixedArray<float, 3>{0.f, 0.f, 0.f}},
                    PhysicsMaterial::ATTR_VISIBLE,
                    std::vector<FixedArray<ColoredVertex<double>, 3>>{merged_tris.begin(), merged_tris.end()},
                    std::vector<FixedArray<ColoredVertex<double>, 2>>{},
                    std::vector<FixedArray<std::vector<BoneWeight>, 3>>{},
                    std::vector<FixedArray<std::vector<BoneWeight>, 2>>{})));
            scene_node_resources.add_companion(
                mesh_resource_name,
                merged_resource_name,
                RenderableResourceFilter{});
        });
}
