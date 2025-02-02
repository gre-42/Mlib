#include "Merge_Textures.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Material/Colormap_With_Modifiers.hpp>
#include <Mlib/Geometry/Material/Merged_Texture_Name.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Geometry/Texture/Uv_Atlas_Tolerance.hpp>
#include <Mlib/Geometry/Texture/Uv_Tile.hpp>
#include <Mlib/Map/Unordered_Map.hpp>
#include <Mlib/Math/Bool.hpp>
#include <Mlib/Render/Modifiers/Merged_Textures_Config.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <list>
#include <map>

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
            UnorderedMap<ColormapWithModifiers, std::list<ColoredVertexArray<CompressedScenePos>*>> merged_filenames;
            auto meshes = scene_node_resource.get_rendering_arrays();
            for (const auto& mesh : meshes) {
                for (const auto& cva : mesh->dcvas) {
                    if (!cva->modifier_backlog.merge_textures) {
                        continue;
                    }
                    if (!any(cva->material.blend_mode & BlendMode::ANY_CONTINUOUS)) {
                        lwarn() << "Attempt to merge object \"" << cva->name << "\", which has a non-continuous material";
                        continue;
                    }
                    // if ((cva->material.wrap_mode_s == WrapMode::REPEAT) ||
                    //     (cva->material.wrap_mode_t == WrapMode::REPEAT))
                    // {
                    //     lwarn() << "Attempt to merge object \"" << cva->name << "\", which has repeating textures";
                    //     continue;
                    // }
                    bool warning_printed = false;
                    auto uv_out_of_bounds = FixedArray<bool, 2>{false, false};
                    for (auto& t : cva->triangles) {
                        for (const auto& v : t.flat_iterable()) {
                            auto too_small = (v.uv < UV_ATLAS_MIN);
                            auto too_large = (v.uv > UV_ATLAS_MAX);
                            if (!warning_printed && any(too_small)) {
                                lwarn() << "UV-coordinates (" << v.uv << ") of object \"" << cva->name << "\" are negative. You can call cleanup_mesh/modulo_uv to fix this. Further warnings suppressed.";
                                warning_printed = true;
                            }
                            if (!warning_printed && any(too_large)) {
                                lwarn() << "UV-coordinates (" << v.uv << ") of object \"" << cva->name << "\" do not permit texture atlas. Did you forget to call cleanup_mesh/modulo_uv?. Further warnings suppressed.";
                                warning_printed = true;
                            }
                            uv_out_of_bounds |= too_small;
                            uv_out_of_bounds |= too_large;
                            if (all(uv_out_of_bounds)) {
                                goto fallback;
                            }
                        }
                    }
                    if (any(uv_out_of_bounds)) {
                        goto fallback;
                    }
                    merged_filenames[MergedTextureName{cva->material}.colormap].push_back(cva.get());
                    continue;
                    fallback:;
                    // if (uv_out_of_bounds(0)) {
                    //     cva->material.wrap_mode_s = WrapMode::REPEAT;
                    // }
                    // if (uv_out_of_bounds(1)) {
                    //     cva->material.wrap_mode_t = WrapMode::REPEAT;
                    // }
                    if (any(cva->material.blend_mode & BlendMode::ANY_CONTINUOUS)) {
                        cva->material.blend_mode = BlendMode::BINARY_05;
                    }
                    cva->morphology.max_triangle_distance = merged_materials_config.max_triangle_distance;
                }
            }
            if (merged_filenames.empty()) {
                return;
            }
            // auto keys = std::views::keys(merged_filenames);
            // auto uv_tiles = rendering_resources.generate_texture_atlas(merged_texture_name, std::set(keys.begin(), keys.end()));
            auto uv_tiles = rendering_resources.generate_auto_texture_atlas(
                merged_materials_config.texture_name,
                merged_filenames.keys(),
                merged_materials_config.mip_level_count);
            // rendering_resources.save_to_file("/tmp/atlas.png", TextureDescriptor{.color = merged_texture_name, .color_mode = ColorMode::RGBA});
            
            std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>> merged_triangles;
            std::list<FixedArray<uint8_t, 3>> merged_discrete_triangle_texture_layers;
            for (const auto& [colormap, cvas] : merged_filenames) {
                const auto& tile = uv_tiles.at(colormap.filename);
                for (const auto& cva : cvas) {
                    for (const auto& tri : cva->triangles) {
                        {
                            auto& mtri = merged_triangles.emplace_back(tri);
                            for (auto& v : mtri.flat_iterable()) {
                                assert_true(all(v.uv >= UV_ATLAS_MIN));
                                assert_true(all(v.uv <= UV_ATLAS_MAX));
                                v.uv = tile.position + v.uv * tile.size;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow"
                                v.color = Colors::from_rgb(cva->material.shading.ambient + cva->material.shading.diffuse);
#pragma GCC diagnostic pop
                            }
                        }
                        {
                            auto& mlay = merged_discrete_triangle_texture_layers.emplace_back(uninitialized);
                            for (auto& v : mlay.flat_iterable()) {
                                v = tile.layer;
                            }
                        }
                    }
                    cva->morphology.physics_material &= ~PhysicsMaterial::ATTR_VISIBLE;
                }
            }
            if (merged_triangles.empty()) {
                return;
            }
            scene_node_resources.add_resource(
                merged_materials_config.resource_name,
                std::make_shared<ColoredVertexArrayResource>(
                    std::make_shared<ColoredVertexArray<CompressedScenePos>>(
                    merged_materials_config.array_name,
                    Material{
                        .blend_mode = merged_materials_config.blend_mode,
                        .continuous_blending_z_order = merged_materials_config.continuous_blending_z_order,
                        .textures_color = {{.texture_descriptor = {.color = merged_materials_config.texture_name}}},
                        .occluded_pass = merged_materials_config.occluded_pass,
                        .occluder_pass = merged_materials_config.occluder_pass,
                        .magnifying_interpolation_mode = InterpolationMode::LINEAR,
                        .aggregate_mode = merged_materials_config.aggregate_mode,
                        .cull_faces = merged_materials_config.cull_faces,
                        .shading {
                            .emissive = OrderableFixedArray<float, 3>{0.f, 0.f, 0.f},
                            .ambient = OrderableFixedArray<float, 3>{1.f, 1.f, 1.f},
                            .diffuse = OrderableFixedArray<float, 3>{0.f, 0.f, 0.f},
                            .specular = OrderableFixedArray<float, 3>{0.f, 0.f, 0.f}}},
                    Morphology{
                        .physics_material = PhysicsMaterial::ATTR_VISIBLE,
                        .max_triangle_distance = merged_materials_config.max_triangle_distance,
                    },
                    ModifierBacklog{},
                    UUVector<FixedArray<ColoredVertex<CompressedScenePos>, 4>>{},
                    UUVector<FixedArray<ColoredVertex<CompressedScenePos>, 3>>(merged_triangles.begin(), merged_triangles.end()),
                    UUVector<FixedArray<ColoredVertex<CompressedScenePos>, 2>>{},
                    UUVector<FixedArray<std::vector<BoneWeight>, 3>>{},
                    UUVector<FixedArray<float, 3>>{},
                    UUVector<FixedArray<uint8_t, 3>>(merged_discrete_triangle_texture_layers.begin(), merged_discrete_triangle_texture_layers.end()),
                    std::vector<UUVector<FixedArray<float, 3, 2>>>{},
                    std::vector<UUVector<FixedArray<float, 3>>>{},
                    UUVector<FixedArray<float, 3>>{})));
            scene_node_resources.add_companion(
                mesh_resource_name,
                merged_materials_config.resource_name,
                RenderableResourceFilter{});
        });
}
