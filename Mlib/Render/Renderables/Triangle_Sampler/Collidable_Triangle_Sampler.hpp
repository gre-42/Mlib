#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Intersection/Bvh_Fwd.hpp>
#include <Mlib/Scene_Precision.hpp>

namespace Mlib {

class TerrainStyles;
struct TerrainTriangles;
class HeterogeneousResource;
enum class UpAxis;

class CollidableTriangleSampler {
public:
    CollidableTriangleSampler(
        const TerrainStyles& terrain_styles,
        ScenePos scale,
        UpAxis up_axis);
    void add_near_hitboxes(
        const TerrainTriangles& tl_terrain,
        const Bvh<CompressedScenePos, 3, FixedArray<CompressedScenePos, 3, 3>>& street_bvh,
        HeterogeneousResource& hri);
    void add_far_hitboxes(
        const TerrainTriangles& tl_terrain,
        const Bvh<CompressedScenePos, 3, FixedArray<CompressedScenePos, 3, 3>>& street_bvh,
        HeterogeneousResource& hri);
private:
    const TerrainStyles& terrain_styles_;
    ScenePos scale_;
    UpAxis up_axis_;
};

}
