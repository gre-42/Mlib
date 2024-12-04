#include "Sat_Normals.hpp"
#include <Mlib/Geometry/Intersection/Collision_Polygon.hpp>
#include <Mlib/Geometry/Intersection/Collision_Ridge.hpp>
#include <Mlib/Geometry/Mesh/IIntersectable_Mesh.hpp>
#include <Mlib/Geometry/Mesh/Sat_Overlap_Combiner.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>

using namespace Mlib;

template <size_t tnvertices>
static void compute_relevant_polys(
    const IIntersectableMesh& mesh0,
    const IIntersectableMesh& mesh1,
    std::vector<const CollisionPolygonSphere<CompressedScenePos, tnvertices>*>& relevant_polys0,
    std::vector<const CollisionPolygonSphere<CompressedScenePos, tnvertices>*>& relevant_polys1)
{
    const std::vector<CollisionPolygonSphere<CompressedScenePos, tnvertices>>& triangles0 = mesh0.get_polygons_sphere<tnvertices>();
    const std::vector<CollisionPolygonSphere<CompressedScenePos, tnvertices>>& triangles1 = mesh1.get_polygons_sphere<tnvertices>();
    relevant_polys0.reserve(triangles0.size());
    relevant_polys1.reserve(triangles1.size());
    for (const auto& t0 : triangles0) {
        if (mesh1.intersects(t0.bounding_sphere) && mesh1.intersects(t0.polygon.plane)) {
            relevant_polys0.push_back(&t0);
        }
    }
    for (const auto& t1 : triangles1) {
        if (mesh0.intersects(t1.bounding_sphere) && mesh0.intersects(t1.polygon.plane)) {
            relevant_polys1.push_back(&t1);
        }
    }
};

template <size_t tnvertices>
static void update_sat(
    const std::vector<const CollisionPolygonSphere<CompressedScenePos, tnvertices>*>& relevant_polys0,
    const std::vector<const CollisionPolygonSphere<CompressedScenePos, tnvertices>*>& relevant_polys1,
    SatOverlapCombiner& sac)
{
    for (const auto& t0 : relevant_polys0) {
        sac.combine_plane(t0->polygon.plane.normal);
    }
    for (const auto& t1 : relevant_polys1) {
        sac.combine_plane(-t1->polygon.plane.normal);
    }
}

void Mlib::get_overlap(
    const IIntersectableMesh& mesh0,
    const IIntersectableMesh& mesh1,
    ScenePos& min_overlap,
    FixedArray<SceneDir, 3>& normal)
{
    std::vector<const CollisionRidgeSphere<CompressedScenePos>*> relevant_edges0;
    std::vector<const CollisionRidgeSphere<CompressedScenePos>*> relevant_edges1;
    std::vector<const CollisionPolygonSphere<CompressedScenePos, 3>*> relevant_triangles0;
    std::vector<const CollisionPolygonSphere<CompressedScenePos, 3>*> relevant_triangles1;
    std::vector<const CollisionPolygonSphere<CompressedScenePos, 4>*> relevant_quads0;
    std::vector<const CollisionPolygonSphere<CompressedScenePos, 4>*> relevant_quads1;
    compute_relevant_polys(mesh0, mesh1, relevant_quads0, relevant_quads1);
    compute_relevant_polys(mesh0, mesh1, relevant_triangles0, relevant_triangles1);
    {
        const auto& edges0 = mesh0.get_ridges_sphere();
        const auto& edges1 = mesh1.get_ridges_sphere();
        relevant_edges0.reserve(edges0.size());
        relevant_edges1.reserve(edges1.size());
        for (const auto& e0 : edges0) {
            if (mesh1.intersects(e0.bounding_sphere)) {
                relevant_edges0.push_back(&e0);
            }
        }
        for (const auto& e1 : edges1) {
            if (mesh0.intersects(e1.bounding_sphere)) {
                relevant_edges1.push_back(&e1);
            }
        }
    }
    SatOverlapCombiner sac{
        mesh0.get_vertices(),
        mesh1.get_vertices()
    };
    update_sat(relevant_quads0, relevant_quads1, sac);
    update_sat(relevant_triangles0, relevant_triangles1, sac);
    for (const auto& e0 : relevant_edges0) {
        for (const auto& e1 : relevant_edges1) {
            sac.combine_ridges(*e0, *e1);
        }
    }
    if (sac.best_min_overlap() == INFINITY) {
        THROW_OR_ABORT("Could not compute overlap, #triangles might be zero");
    }
    min_overlap = sac.best_min_overlap();
    normal = sac.best_normal();
}

void SatTracker::get_collision_plane(
    const IIntersectableMesh& mesh0,
    const IIntersectableMesh& mesh1,
    ScenePos& min_overlap,
    FixedArray<SceneDir, 3>& normal) const
{
    if (&mesh0 == &mesh1) {
        THROW_OR_ABORT("Mesh compared to itself");
    }
    if (&mesh0 > &mesh1) {
        get_collision_plane(mesh1, mesh0, min_overlap, normal);
        normal = -normal;
        return;
    }
    if (collision_planes_.find(&mesh0) == collision_planes_.end()) {
        collision_planes_.insert(std::make_pair(
            &mesh0,
            std::map<const IIntersectableMesh*,
                std::pair<ScenePos, FixedArray<SceneDir, 3>>>()));
    }
    auto& collision_planes_m0 = collision_planes_.at(&mesh0);
    if (collision_planes_m0.find(&mesh1) == collision_planes_m0.end()) {
        ScenePos min_overlap;
        FixedArray<SceneDir, 3> normal = uninitialized;
        get_overlap(mesh0, mesh1, min_overlap, normal);
        // lerr() << "min_overlap " << min_overlap << " best_triangle " << best_triangle << " best normal " << triangle_normal(best_triangle);
        collision_planes_m0.insert(std::make_pair(&mesh1, std::make_pair(min_overlap, normal)));
    }
    const auto& res = collision_planes_m0.at(&mesh1);
    min_overlap = res.first;
    normal = res.second;
    // auto res = collision_normals_.at(o0).at(o1) - collision_normals_.at(o1).at(o0);
    // res /= std::sqrt(sum(squared(res)));
    // return res;
}
