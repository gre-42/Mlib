#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Terrain_Style.hpp>

namespace Mlib {

struct TriangleSamplerResourceConfig {
    TerrainStyleConfig near_grass_terrain_style_config{ .much_near_distance = 2 };
    TerrainStyleConfig far_grass_terrain_style_config{ .much_near_distance = 20 };
    TerrainStyleConfig near_wayside1_grass_terrain_style_config{ .much_near_distance = 1 };
    TerrainStyleConfig near_wayside2_grass_terrain_style_config{ .much_near_distance = 1 };
    TerrainStyleConfig near_flowers_terrain_style_config{ .much_near_distance = 2 };
    TerrainStyleConfig far_flowers_terrain_style_config{ .much_near_distance = 5 };
    TerrainStyleConfig near_trees_terrain_style_config{ .much_near_distance = 5 };
    TerrainStyleConfig far_trees_terrain_style_config{ .much_near_distance = 20 };
    TerrainStyleConfig no_grass_decals_terrain_style_config{ .much_near_distance = 10 };
};

}
