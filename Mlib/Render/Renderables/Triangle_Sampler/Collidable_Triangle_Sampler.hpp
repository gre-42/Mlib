#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Pos.hpp>

namespace Mlib {

class TerrainStyles;
struct TerrainTriangles;
class HeterogeneousResource;
template <class TData, class TPayload, size_t tndim>
class Bvh;
enum class UpAxis;

class CollidableTriangleSampler {
public:
    CollidableTriangleSampler(
        const TerrainStyles& terrain_styles,
        ScenePos scale,
        UpAxis up_axis);
    void add_near_hitboxes(
        const TerrainTriangles& tl_terrain,
        const Bvh<ScenePos, FixedArray<ScenePos, 3, 3>, 3>& street_bvh,
        HeterogeneousResource& hri);
    void add_far_hitboxes(
        const TerrainTriangles& tl_terrain,
        const Bvh<ScenePos, FixedArray<ScenePos, 3, 3>, 3>& street_bvh,
        HeterogeneousResource& hri);
private:
    const TerrainStyles& terrain_styles_;
    ScenePos scale_;
    UpAxis up_axis_;
};

}
