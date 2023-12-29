#include "Sat_Normals.hpp"
#include <Mlib/Geometry/Intersection/Collision_Line.hpp>
#include <Mlib/Geometry/Intersection/Collision_Polygon.hpp>
#include <Mlib/Geometry/Mesh/IIntersectable_Mesh.hpp>
#include <Mlib/Geometry/Mesh/Sat_Overlap.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>

using namespace Mlib;

template <size_t tnvertices>
static void compute_relevant_polys(
    const IIntersectableMesh& mesh0,
    const IIntersectableMesh& mesh1,
    std::vector<const CollisionPolygonSphere<tnvertices>*>& relevant_polys0,
    std::vector<const CollisionPolygonSphere<tnvertices>*>& relevant_polys1,
    CollisionVertices& vertices0,
    CollisionVertices& vertices1)
{
    const std::vector<CollisionPolygonSphere<tnvertices>>& triangles0 = mesh0.get_polygons_sphere<tnvertices>();
    const std::vector<CollisionPolygonSphere<tnvertices>>& triangles1 = mesh1.get_polygons_sphere<tnvertices>();
    relevant_polys0.reserve(triangles0.size());
    relevant_polys1.reserve(triangles1.size());
    for (const auto& t0 : triangles0) {
        if (mesh1.intersects(t0.bounding_sphere) && mesh1.intersects(t0.polygon.plane())) {
            relevant_polys0.push_back(&t0);
        }
        vertices0.insert(t0.corners);
    }
    for (const auto& t1 : triangles1) {
        if (mesh0.intersects(t1.bounding_sphere) && mesh0.intersects(t1.polygon.plane())) {
            relevant_polys1.push_back(&t1);
        }
        vertices1.insert(t1.corners);
    }
};

template <size_t tnvertices>
static void update_sat(
    const std::vector<const CollisionPolygonSphere<tnvertices>*>& relevant_polys0,
    const std::vector<const CollisionPolygonSphere<tnvertices>*>& relevant_polys1,
    const CollisionVertices& vertices0,
    const CollisionVertices& vertices1,
    double& best_min_overlap,
    FixedArray<double, 3>& best_normal)
{
    for (const auto& t0 : relevant_polys0) {
        double sat_overl = sat_overlap_signed(
            t0->polygon.plane().normal,
            vertices0,
            vertices1);
        if (sat_overl < best_min_overlap) {
            best_min_overlap = sat_overl;
            best_normal = t0->polygon.plane().normal;
        }
    }
    for (const auto& t1 : relevant_polys1) {
        double sat_overl = sat_overlap_signed(
            t1->polygon.plane().normal,
            vertices1,
            vertices0);
        if (sat_overl < best_min_overlap) {
            best_min_overlap = sat_overl;
            best_normal = -t1->polygon.plane().normal;
        }
    }
}

void SatTracker::get_collision_plane(
    const IIntersectableMesh& mesh0,
    const IIntersectableMesh& mesh1,
    double& min_overlap__,
    FixedArray<double, 3>& normal__) const
{
    if (&mesh0 == &mesh1) {
        THROW_OR_ABORT("Mesh compared to itself");
    }
    if (&mesh0 > &mesh1) {
        get_collision_plane(mesh1, mesh0, min_overlap__, normal__);
        normal__ = -normal__;
        return;
    }
    if (collision_planes_.find(&mesh0) == collision_planes_.end()) {
        collision_planes_.insert(std::make_pair(
            &mesh0,
            std::map<const IIntersectableMesh*,
                std::pair<double, FixedArray<double, 3>>>()));
    }
    auto& collision_planes_m0 = collision_planes_.at(&mesh0);
    if (collision_planes_m0.find(&mesh1) == collision_planes_m0.end()) {
        #define min_overlap__ DO_NOT_USE_ME
        #define normal__ DO_NOT_USE_ME
        double best_min_overlap = INFINITY;
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
        FixedArray<double, 3> best_normal;
        #pragma GCC diagnostic pop
        CollisionVertices vertices0;
        CollisionVertices vertices1;
        std::vector<const CollisionLineSphere*> relevant_edges0;
        std::vector<const CollisionLineSphere*> relevant_edges1;
        std::vector<const CollisionPolygonSphere<3>*> relevant_triangles0;
        std::vector<const CollisionPolygonSphere<3>*> relevant_triangles1;
        std::vector<const CollisionPolygonSphere<4>*> relevant_quads0;
        std::vector<const CollisionPolygonSphere<4>*> relevant_quads1;
        compute_relevant_polys(mesh0, mesh1, relevant_quads0, relevant_quads1, vertices0, vertices1);
        compute_relevant_polys(mesh0, mesh1, relevant_triangles0, relevant_triangles1, vertices0, vertices1);
        {
            const std::vector<CollisionLineSphere>& edges0 = mesh0.get_edges_sphere();
            const std::vector<CollisionLineSphere>& edges1 = mesh1.get_edges_sphere();
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
        update_sat(relevant_quads0, relevant_quads1, vertices0, vertices1, best_min_overlap, best_normal);
        update_sat(relevant_triangles0, relevant_triangles1, vertices0, vertices1, best_min_overlap, best_normal);
        for (const auto& e0 : relevant_edges0) {
            for (const auto& e1 : relevant_edges1) {
                auto n = cross(e0->line(1) - e0->line(0), e1->line(1) - e1->line(0));
                double l2 = sum(squared(n));
                if (l2 < 1e-6) {
                    continue;
                }
                n /= std::sqrt(l2);
                double overlap0;
                double overlap1;
                sat_overlap_unsigned(
                    n,
                    vertices0,
                    vertices1,
                    overlap0,
                    overlap1);
                if (overlap0 < overlap1) {
                    if (overlap0 < best_min_overlap) {
                        best_min_overlap = overlap0;
                        best_normal = -n;
                    }
                } else {
                    if (overlap1 < best_min_overlap) {
                        best_min_overlap = overlap1;
                        best_normal = n;
                    }
                }
            }
        }
        if (best_min_overlap == INFINITY) {
            THROW_OR_ABORT("Could not compute overlap, #triangles might be zero");
        }
        // std::cerr << "min_overlap " << min_overlap << " best_triangle " << best_triangle << " best normal " << triangle_normal(best_triangle) << std::endl;
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
        collision_planes_m0.insert(std::make_pair(&mesh1, std::make_pair(best_min_overlap, best_normal)));
        #pragma GCC diagnostic pop
        #undef min_overlap__
        #undef normal__
    }
    const auto& res = collision_planes_m0.at(&mesh1);
    min_overlap__ = res.first;
    normal__ = res.second;
    // auto res = collision_normals_.at(o0).at(o1) - collision_normals_.at(o1).at(o0);
    // res /= std::sqrt(sum(squared(res)));
    // return res;
}
