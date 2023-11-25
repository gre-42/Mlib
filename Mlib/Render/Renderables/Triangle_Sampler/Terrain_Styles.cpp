#include "Terrain_Styles.hpp"
#include <Mlib/Render/Renderables/Triangle_Sampler/Triangle_Sampler_Resource_Config.hpp>

using namespace Mlib;

TerrainStyles::TerrainStyles() = default;

TerrainStyles::~TerrainStyles() = default;

TerrainStyles::TerrainStyles(const TriangleSamplerResourceConfig& config)
    : street_mud_terrain_style{ config.street_mud_config }
    , path_mud_terrain_style{ config.path_mud_config }
    , near_grass_terrain_style { config.near_grass_terrain_style_config }
    , far_grass_terrain_style{ config.far_grass_terrain_style_config }
    , near_wayside1_grass_terrain_style{ config.near_wayside1_grass_terrain_style_config }
    , near_wayside2_grass_terrain_style{ config.near_wayside2_grass_terrain_style_config }
    , near_flowers_terrain_style{ config.near_flowers_terrain_style_config }
    , far_flowers_terrain_style{ config.far_flowers_terrain_style_config }
    , near_trees_terrain_style{ config.near_trees_terrain_style_config }
    , far_trees_terrain_style{ config.far_trees_terrain_style_config }
    , no_grass_decals_terrain_style{ config.no_grass_decals_terrain_style_config }
{}

bool TerrainStyles::requires_renderer() const {
    return
        street_mud_terrain_style.is_visible() ||
        path_mud_terrain_style.is_visible() ||
        near_grass_terrain_style.is_visible() ||
        near_wayside1_grass_terrain_style.is_visible() ||
        near_wayside2_grass_terrain_style.is_visible() ||
        near_flowers_terrain_style.is_visible() ||
        near_trees_terrain_style.is_visible() ||
        no_grass_decals_terrain_style.is_visible();
}
