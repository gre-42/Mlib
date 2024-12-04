#include "Sat_Overlap2.hpp"
#include <Mlib/Geometry/Intersection/Collision_Line.hpp>
#include <Mlib/Geometry/Intersection/Collision_Polygon.hpp>
#include <Mlib/Geometry/Intersection/Collision_Ridge.hpp>
#include <Mlib/Geometry/Mesh/Collision_Vertices.hpp>
#include <Mlib/Geometry/Mesh/IIntersectable_Mesh.hpp>
#include <Mlib/Geometry/Mesh/Sat_Overlap_Combiner.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>

using namespace Mlib;

template <size_t tnvertices>
static void compute_relevant_polygons(
    const IIntersectableMesh& mesh0,
    const CollisionRidgeSphere<CompressedScenePos>& e1,
    std::vector<const CollisionPolygonSphere<CompressedScenePos, tnvertices>*>& relevant_polygons0)
{
    const std::vector<CollisionPolygonSphere<CompressedScenePos, tnvertices>>& triangles0 = mesh0.get_polygons_sphere<tnvertices>();
    relevant_polygons0.reserve(triangles0.size());
    for (const auto& t0 : triangles0) {
        if (e1.bounding_sphere.intersects(t0.bounding_sphere) &&
            e1.bounding_sphere.intersects(t0.polygon.plane))
        {
            relevant_polygons0.push_back(&t0);
        }
    }
}

void Mlib::get_overlap2(
    const IIntersectableMesh& mesh0,
    const CollisionRidgeSphere<CompressedScenePos>& e1,
    ScenePos max_keep_normal,
    ScenePos& min_overlap,
    FixedArray<SceneDir, 3>& normal)
{
    CollisionVertices vertices1;
    std::vector<const CollisionRidgeSphere<CompressedScenePos>*> relevant_edges0;
    std::vector<const CollisionPolygonSphere<CompressedScenePos, 4>*> relevant_quads0;
    std::vector<const CollisionPolygonSphere<CompressedScenePos, 3>*> relevant_triangles0;
    compute_relevant_polygons(mesh0, e1, relevant_quads0);
    compute_relevant_polygons(mesh0, e1, relevant_triangles0);
    {
        const std::vector<CollisionRidgeSphere<CompressedScenePos>>& edges0 = mesh0.get_ridges_sphere();
        relevant_edges0.reserve(edges0.size());
        for (const auto& e0 : edges0) {
            if (e1.bounding_sphere.intersects(e0.bounding_sphere)) {
                relevant_edges0.push_back(&e0);
            }
        }
    }
    vertices1.insert(e1.edge);

    SatOverlapCombiner sac{
        mesh0.get_vertices(),
        vertices1.get()
    };

    sac.combine_sticky_ridge(e1, max_keep_normal);
    // for (const auto& vn1 : e1.vertex_normals.row_iterable()) {
    //     sac.combine_plane(vn1.casted<ScenePos>());
    // }

    for (const auto& q0 : relevant_quads0) {
        sac.combine_plane(q0->polygon.plane.normal);
        // for (const auto& vn0 : q0->vertex_normals.row_iterable()) {
        //     sac.combine_plane(vn0.casted<ScenePos>());
        // }
    }
    for (const auto& t0 : relevant_triangles0) {
        sac.combine_plane(t0->polygon.plane.normal);
        // for (const auto& vn0 : t0->vertex_normals.row_iterable()) {
        //     sac.combine_plane(vn0.casted<ScenePos>());
        // }
    }
    for (const auto& e0 : relevant_edges0) {
        sac.combine_ridges(*e0, e1);
    }
    // if (best_min_overlap == INFINITY) {
    //     THROW_OR_ABORT("Could not compute overlap, #triangles might be zero, or edge angles are not correct");
    // }
    // lerr() << "min_overlap " << min_overlap << " best_triangle " << best_triangle << " best normal " << triangle_normal(best_triangle);
    min_overlap = sac.best_min_overlap();
    normal = sac.best_normal();
}
