#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <cstdint>
#include <memory>

namespace Mlib {

template <class TPos>
class ColoredVertexArray;
enum class PhysicsMaterial : uint32_t;

template <class TPos>
class CleanupMesh {
public:
    CleanupMesh();
    ~CleanupMesh();
    void operator () (
        ColoredVertexArray<TPos>& cva,
        PhysicsMaterial min_distance_material_filter,
        const TPos& min_vertex_distance,
        bool modulo_uv);
private:
    PointWithoutPayloadVectorBvh<TPos, 3> bvh_;
};

}
