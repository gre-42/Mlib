#pragma once
#include <Mlib/Render/Renderables/Triangle_Sampler/Terrain_Style.hpp>

namespace Mlib {

struct TriangleSamplerResourceConfig;

class TerrainStyles {
public:
    TerrainStyles();
    ~TerrainStyles();
    explicit TerrainStyles(const TriangleSamplerResourceConfig& config);

    bool requires_renderer() const;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(street_mud_terrain_style);
        archive(path_mud_terrain_style);
        archive(near_grass_terrain_style);
        archive(far_grass_terrain_style);
        archive(near_wayside1_grass_terrain_style);
        archive(near_wayside2_grass_terrain_style);
        archive(near_flowers_terrain_style);
        archive(far_flowers_terrain_style);
        archive(near_trees_terrain_style);
        archive(far_trees_terrain_style);
        archive(no_grass_decals_terrain_style);
    }

    TerrainStyle street_mud_terrain_style{ TerrainStyleConfig{ .much_near_distance = 2 } };
    TerrainStyle path_mud_terrain_style{ TerrainStyleConfig{ .much_near_distance = 2 } };
    TerrainStyle near_grass_terrain_style{ TerrainStyleConfig{ .much_near_distance = 2 } };
    TerrainStyle far_grass_terrain_style{ TerrainStyleConfig{ .much_near_distance = 2 } };
    TerrainStyle near_wayside1_grass_terrain_style{ TerrainStyleConfig{ .much_near_distance = 1 } };
    TerrainStyle near_wayside2_grass_terrain_style{ TerrainStyleConfig{ .much_near_distance = 2 } };
    TerrainStyle near_flowers_terrain_style{ TerrainStyleConfig{ .much_near_distance = 2 } };
    TerrainStyle far_flowers_terrain_style{ TerrainStyleConfig{ .much_near_distance = 5 } };
    TerrainStyle near_trees_terrain_style{ TerrainStyleConfig{ .much_near_distance = 5, .size_classification = SizeClassification::LARGE } };
    TerrainStyle far_trees_terrain_style{ TerrainStyleConfig{ .much_near_distance = 20, .size_classification = SizeClassification::LARGE } };
    TerrainStyle no_grass_decals_terrain_style{ TerrainStyleConfig{ .much_near_distance = 10 } };
};

}
