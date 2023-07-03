#pragma once
#include <Mlib/Render/Renderables/Triangle_Sampler/Terrain_Style.hpp>

namespace Mlib {

struct TerrainTriangles;
struct TriangleSamplerResourceConfig;
class HeterogeneousResource;
template <class TData, class TPayload, size_t tndim>
class Bvh;

class TerrainStyles {
public:
    TerrainStyles();
    explicit TerrainStyles(const TriangleSamplerResourceConfig& config);

    void add_near_hitboxes(
        const TerrainTriangles& tl_terrain,
        const Bvh<double, FixedArray<FixedArray<double, 3>, 3>, 3>& street_bvh,
        HeterogeneousResource& hri);
    void add_far_hitboxes(
        const TerrainTriangles& tl_terrain,
        const Bvh<double, FixedArray<FixedArray<double, 3>, 3>, 3>& street_bvh,
        HeterogeneousResource& hri);
    bool requires_renderer() const;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(scale_);
        archive(near_grass_terrain_style_);
        archive(far_grass_terrain_style_);
        archive(near_wayside1_grass_terrain_style_);
        archive(near_wayside2_grass_terrain_style_);
        archive(near_flowers_terrain_style_);
        archive(far_flowers_terrain_style_);
        archive(near_trees_terrain_style_);
        archive(far_trees_terrain_style_);
        archive(no_grass_decals_terrain_style_);
    }

    TerrainStyle near_grass_terrain_style_{ TerrainStyleConfig{ .much_near_distance = 2 } };
    TerrainStyle far_grass_terrain_style_{ TerrainStyleConfig{ .much_near_distance = 2 } };
    TerrainStyle near_wayside1_grass_terrain_style_{ TerrainStyleConfig{ .much_near_distance = 1 } };
    TerrainStyle near_wayside2_grass_terrain_style_{ TerrainStyleConfig{ .much_near_distance = 2 } };
    TerrainStyle near_flowers_terrain_style_{ TerrainStyleConfig{ .much_near_distance = 2 } };
    TerrainStyle far_flowers_terrain_style_{ TerrainStyleConfig{ .much_near_distance = 5 } };
    TerrainStyle near_trees_terrain_style_{ TerrainStyleConfig{ .much_near_distance = 5 } };
    TerrainStyle far_trees_terrain_style_{ TerrainStyleConfig{ .much_near_distance = 20 } };
    TerrainStyle no_grass_decals_terrain_style_{ TerrainStyleConfig{ .much_near_distance = 10 } };
private:
    float scale_;
};

}
