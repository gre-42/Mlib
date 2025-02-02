#include "Cleanup_Mesh.hpp"
#include <Mlib/Geometry/Mesh/Cleanup/Merge_Neighboring_Points.hpp>
#include <Mlib/Geometry/Mesh/Cleanup/Modulo_Uv.hpp>
#include <Mlib/Geometry/Mesh/Cleanup/Remove_Degenerate_Triangles.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>

using namespace Mlib;

template <class TPos>
CleanupMesh<TPos>::CleanupMesh()
    : bvh_{ FixedArray<TPos, 3>{(TPos)0.1f, (TPos)0.1f, (TPos)0.1f}, 17 }
{}

template <class TPos>
CleanupMesh<TPos>::~CleanupMesh() = default;

template <class TPos>
void CleanupMesh<TPos>::operator () (
    ColoredVertexArray<TPos>& cva,
    PhysicsMaterial min_distance_material_filter,
    const TPos& min_vertex_distance,
    bool modulo_uv)
{
    if ((min_vertex_distance != (TPos)0.f) &&
        ((cva.morphology.physics_material & min_distance_material_filter) == min_distance_material_filter))
    {
        merge_neighboring_points<TPos>(cva, bvh_, min_vertex_distance);
    }
    remove_degenerate_triangles(cva);
    // remove_duplicate_triangles(cva);
    // remove_triangles_with_opposing_normals(cva);
    if (modulo_uv) {
        Mlib::modulo_uv(cva);
    }
}

namespace Mlib {

template class CleanupMesh<float>;
template class CleanupMesh<CompressedScenePos>;

}
