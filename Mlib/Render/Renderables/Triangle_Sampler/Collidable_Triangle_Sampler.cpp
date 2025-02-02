#include "Collidable_Triangle_Sampler.hpp"
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Sample_Triangle_Interior_Instances.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Terrain_Styles.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Terrain_Triangles.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Terrain_Type.hpp>
#include <Mlib/Render/Resources/Heterogeneous_Resource.hpp>
#include <Mlib/Scene_Graph/Descriptors/Resource_Instance_Descriptor.hpp>
#include <Mlib/Scene_Graph/Resources/Batch_Resource_Instantiator.hpp>
#include <list>
#include <memory>

using namespace Mlib;

CollidableTriangleSampler::CollidableTriangleSampler(
    const TerrainStyles& terrain_styles,
    ScenePos scale,
    UpAxis up_axis)
    : terrain_styles_{ terrain_styles }
    , scale_{ scale }
    , up_axis_{ up_axis }
{}

void CollidableTriangleSampler::add_near_hitboxes(
    const TerrainTriangles& tl_terrain,
    const Bvh<CompressedScenePos, 3, FixedArray<CompressedScenePos, 3, 3>>& street_bvh,
    HeterogeneousResource& hri)
{
    std::list<std::pair<const TerrainStyle&, const UUList<FixedArray<ColoredVertex<CompressedScenePos>, 3>>*>> grass_triangles;
    if (const auto& style = terrain_styles_.near_wayside1_grass_terrain_style; style.is_visible()) {
        if (auto tris = tl_terrain.wayside1_grass; tris != nullptr) {
            grass_triangles.push_back({ style, tris });
        }
    }
    if (const auto& style = terrain_styles_.near_wayside2_grass_terrain_style; style.is_visible()) {
        if (auto tris = tl_terrain.wayside2_grass; tris != nullptr) {
            grass_triangles.push_back({ style, tris });
        }
    }
    if (const auto& style = terrain_styles_.near_trees_terrain_style; style.is_visible()) {
        if (auto tris = tl_terrain.trees; tris != nullptr) {
            grass_triangles.push_back({ style, tris });
        }
    }
    auto add_triangles = [this, &street_bvh, &hri](
        const UUList<FixedArray<ColoredVertex<CompressedScenePos>, 3>>& gtl,
        const TerrainStyle& terrain_style)
    {
        TriangleInteriorInstancesSampler tiis{
            terrain_style,
            scale_,
            up_axis_,
            &street_bvh,
            terrain_style.foliagemap(),
            terrain_style.config.foliagemap_scale,
            terrain_style.mudmap()};
        unsigned int seed = 0;
        for (const auto& t : gtl) {
            ++seed;
            tiis.sample_triangle(
                t,
                seed,
                [&hri](
                    const FixedArray<CompressedScenePos, 3>& p,
                    const ParsedResourceName& prn)
                {
                    if (!prn.hitbox.empty()) {
                        hri.bri->add_hitbox(
                            prn.hitbox,
                            ResourceInstanceDescriptor{
                                .position = p,
                                .yangle = 0.f,
                                .scale = 1.f,
                                .billboard_id = BILLBOARD_ID_NONE});
                    }
                });
        }
    };
    for (const auto& [style, lst] : grass_triangles) {
        add_triangles(*lst, style);
    }
}

void CollidableTriangleSampler::add_far_hitboxes(
    const TerrainTriangles& tl_terrain,
    const Bvh<CompressedScenePos, 3, FixedArray<CompressedScenePos, 3, 3>>& street_bvh,
    HeterogeneousResource& hri)
{
    std::list<std::pair<const TerrainStyle&, const UUList<FixedArray<ColoredVertex<CompressedScenePos>, 3>>*>> grass_triangles;
    if (const auto& style = terrain_styles_.far_grass_terrain_style; style.is_visible()) {
        if (auto tris = tl_terrain.grass; tris != nullptr) {
            grass_triangles.push_back({ style, tris });
        }
    }
    if (const auto& style = terrain_styles_.far_grass_terrain_style; style.is_visible()) {
        if (auto tris = tl_terrain.wayside1_grass; tris != nullptr) {
            grass_triangles.push_back({ style, tris });
        }
    }
    if (const auto& style = terrain_styles_.far_grass_terrain_style; style.is_visible()) {
        if (auto tris = tl_terrain.wayside2_grass; tris != nullptr) {
            grass_triangles.push_back({ style, tris });
        }
    }
    if (const auto& style = terrain_styles_.far_flowers_terrain_style; style.is_visible()) {
        if (auto tris = tl_terrain.flowers; tris != nullptr) {
            grass_triangles.push_back({ style, tris });
        }
    }
    if (const auto& style = terrain_styles_.far_trees_terrain_style; style.is_visible()) {
        if (auto tris = tl_terrain.trees; tris != nullptr) {
            grass_triangles.push_back({ style, tris });
        }
    }
    auto add_triangles = [this, &street_bvh, &hri](
        const UUList<FixedArray<ColoredVertex<CompressedScenePos>, 3>>& gtl,
        const TerrainStyle& terrain_style)
    {
        TriangleInteriorInstancesSampler tiis{
            terrain_style,
            scale_,
            up_axis_,
            &street_bvh,
            terrain_style.foliagemap(),
            terrain_style.config.foliagemap_scale,
            terrain_style.mudmap()};
        unsigned int seed = 8579;
        for (const auto& t : gtl) {
            ++seed;
            tiis.sample_triangle(
                t,
                seed,
                [&hri](
                    const FixedArray<CompressedScenePos, 3>& p,
                    const ParsedResourceName& prn)
                {
                    if (!prn.hitbox.empty()) {
                        hri.bri->add_hitbox(
                            prn.hitbox,
                            ResourceInstanceDescriptor{
                                .position = p,
                                .yangle = 0.f,
                                .scale = 1.f,
                                .billboard_id = BILLBOARD_ID_NONE});
                    }
                    hri.bri->add_parsed_resource_name(
                        p,
                        prn,
                        0.f,   // yangle
                        1.f);  // scale
                });
        }
    };
    for (const auto& [style, lst] : grass_triangles) {
        add_triangles(*lst, style);
    }
}
