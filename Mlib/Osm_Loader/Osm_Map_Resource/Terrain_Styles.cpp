#include "Terrain_Styles.hpp"
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Resource_Config.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Triangle_Lists.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Sample_Triangle_Interior_Instances.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Terrain_Type.hpp>
#include <Mlib/Render/Resources/Heterogeneous_Resource.hpp>
#include <Mlib/Scene_Graph/Descriptors/Resource_Instance_Descriptor.hpp>
#include <Mlib/Scene_Graph/Resources/Batch_Resource_Instantiator.cpp>
#include <list>
#include <memory>

using namespace Mlib;

TerrainStyles::TerrainStyles()
: scale_{NAN}
{}

TerrainStyles::TerrainStyles(const OsmResourceConfig& config)
: near_grass_terrain_style_{ config.near_grass_terrain_style_config },
  far_grass_terrain_style_{ config.far_grass_terrain_style_config },
  near_wayside1_grass_terrain_style_{ config.near_wayside1_grass_terrain_style_config },
  near_wayside2_grass_terrain_style_{ config.near_wayside2_grass_terrain_style_config },
  near_flowers_terrain_style_{ config.near_flowers_terrain_style_config },
  far_flowers_terrain_style_{ config.far_flowers_terrain_style_config },
  near_trees_terrain_style_{ config.near_trees_terrain_style_config },
  far_trees_terrain_style_{ config.far_trees_terrain_style_config },
  no_grass_decals_terrain_style_{ config.no_grass_decals_terrain_style_config },
  scale_{config.scale}
{}

void TerrainStyles::add_near_hitboxes(
    const TerrainTypeTriangleList& tl_terrain,
    const Bvh<double, FixedArray<FixedArray<double, 3>, 3>, 3>& street_bvh,
    HeterogeneousResource& hri)
{
    std::list<std::pair<const TerrainStyle&, std::shared_ptr<TriangleList<double>>>> grass_triangles;
    if (auto tit = tl_terrain.map().find(TerrainType::WAYSIDE1_GRASS); tit != tl_terrain.map().end())
    {
        grass_triangles.push_back({ near_wayside1_grass_terrain_style_, tit->second });
    }
    if (auto tit = tl_terrain.map().find(TerrainType::WAYSIDE2_GRASS); tit != tl_terrain.map().end())
    {
        grass_triangles.push_back({ near_wayside2_grass_terrain_style_, tit->second });
    }
    if (auto tit = tl_terrain.map().find(TerrainType::TREES); tit != tl_terrain.map().end())
    {
        if (near_trees_terrain_style_.is_visible()) {
            grass_triangles.push_back({ near_trees_terrain_style_, tit->second });
        }
    }
    auto add_triangles = [this, &street_bvh, &hri](
        const TriangleList<double>& gtl,
        const TerrainStyle& terrain_style)
    {
        TriangleInteriorInstancesSampler tiis{
            terrain_style,
            scale_,
            &street_bvh,
            terrain_style.foliagemap(),
            terrain_style.config.foliagemap_scale};
        unsigned int seed = 0;
        for (const auto& t : gtl.triangles_) {
            ++seed;
            tiis.sample_triangle(
                t,
                seed,
                [&hri](
                    const FixedArray<double, 3>& p,
                    const ParsedResourceName& prn)
                {
                    if (!prn.hitbox.empty()) {
                        hri.bri->add_hitbox(
                            prn.hitbox,
                            ResourceInstanceDescriptor{
                                .position = p,
                                .yangle = 0.f,
                                .scale = 1.f,
                                .billboard_id = UINT32_MAX});
                    }
                });
        }
    };
    for (const auto& [style, lst] : grass_triangles) {
        add_triangles(*lst, style);
    }
}

void TerrainStyles::add_far_hitboxes(
    const TerrainTypeTriangleList& tl_terrain,
    const Bvh<double, FixedArray<FixedArray<double, 3>, 3>, 3>& street_bvh,
    HeterogeneousResource& hri)
{
    std::list<std::pair<const TerrainStyle&, std::shared_ptr<TriangleList<double>>>> grass_triangles;
    if (far_grass_terrain_style_.config.is_visible()) {
        if (auto tit = tl_terrain.map().find(TerrainType::GRASS); tit != tl_terrain.map().end())
        {
            grass_triangles.push_back({ far_grass_terrain_style_, tit->second });
        }
    }
    if (far_grass_terrain_style_.config.is_visible()) {
        if (auto tit = tl_terrain.map().find(TerrainType::WAYSIDE1_GRASS); tit != tl_terrain.map().end())
        {
            grass_triangles.push_back({ far_grass_terrain_style_, tit->second });
        }
    }
    if (far_grass_terrain_style_.config.is_visible()) {
        if (auto tit = tl_terrain.map().find(TerrainType::WAYSIDE2_GRASS); tit != tl_terrain.map().end())
        {
            grass_triangles.push_back({ far_grass_terrain_style_, tit->second });
        }
    }
    if (far_flowers_terrain_style_.config.is_visible()) {
        if (auto tit = tl_terrain.map().find(TerrainType::FLOWERS); tit != tl_terrain.map().end())
        {
            grass_triangles.push_back({ far_flowers_terrain_style_, tit->second });
        }
    }
    if (far_trees_terrain_style_.config.is_visible()) {
        if (auto tit = tl_terrain.map().find(TerrainType::TREES); tit != tl_terrain.map().end())
        {
            grass_triangles.push_back({ far_trees_terrain_style_, tit->second });
        }
    }
    auto add_triangles = [this, &street_bvh, &hri](
        const TriangleList<double>& gtl,
        const TerrainStyle& terrain_style)
    {
        TriangleInteriorInstancesSampler tiis{
            terrain_style,
            scale_,
            &street_bvh,
            terrain_style.foliagemap(),
            terrain_style.config.foliagemap_scale};
        unsigned int seed = 8579;
        for (const auto& t : gtl.triangles_) {
            ++seed;
            tiis.sample_triangle(
                t,
                seed,
                [&hri](
                    const FixedArray<double, 3>& p,
                    const ParsedResourceName& prn)
                {
                    if (!prn.hitbox.empty()) {
                        hri.bri->add_hitbox(
                            prn.hitbox,
                            ResourceInstanceDescriptor{
                                .position = p,
                                .yangle = 0.f,
                                .scale = 1.f,
                                .billboard_id = UINT32_MAX});
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

bool TerrainStyles::requires_renderer() const {
    return
        near_grass_terrain_style_.is_visible() ||
        near_wayside1_grass_terrain_style_.is_visible() ||
        near_wayside2_grass_terrain_style_.is_visible() ||
        near_flowers_terrain_style_.is_visible() ||
        near_trees_terrain_style_.is_visible() ||
        no_grass_decals_terrain_style_.is_visible();
}
