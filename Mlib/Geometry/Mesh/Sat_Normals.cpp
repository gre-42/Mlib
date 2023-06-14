#include "Sat_Normals.hpp"
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Collision_Triangle.hpp>
#include <Mlib/Geometry/Mesh/IIntersectable_Mesh.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <set>

using namespace Mlib;

class CollisionVertices {
public:
    void insert(const FixedArray<FixedArray<double, 3>, 3>& tri) {
        insert(tri(0));
        insert(tri(1));
        insert(tri(2));
    }
    void insert(const FixedArray<double, 3>& a) {
        vertices_.insert(OrderableFixedArray{a});
    }
    auto begin() const {
        return vertices_.begin();
    }
    auto end() const {
        return vertices_.end();
    }
private:
    std::set<OrderableFixedArray<double, 3>> vertices_;
};

class CollisionEdges {
public:
    void insert(const FixedArray<FixedArray<double, 3>, 3>& tri) {
        insert(tri(0), tri(1));
        insert(tri(1), tri(2));
        insert(tri(2), tri(0));
    }
    void insert(const FixedArray<double, 3>& a, const FixedArray<double, 3>& b) {
        if (OrderableFixedArray{a} < OrderableFixedArray{b}) {
            edges_.insert({OrderableFixedArray{a - b}});
        } else {
            edges_.insert({OrderableFixedArray{b - a}});
        }
    }
    auto begin() const {
        return edges_.begin();
    }
    auto end() const {
        return edges_.end();
    }
private:
    std::set<OrderableFixedArray<double, 3>> edges_;
};

static double sat_overlap_signed(
    const FixedArray<double, 3>& n,
    const CollisionVertices& vertices0,
    const CollisionVertices& vertices1)
{
    double max0 = -INFINITY;
    double min1 = INFINITY;
    for (const auto& v : vertices0) {
        double s = dot0d(v, n);
        max0 = std::max(max0, s);
    }
    for (const auto& v : vertices1) {
        double s = dot0d(v, n);
        min1 = std::min(min1, s);
    }
    // o0 -> normal | o1
    // o0_min .. o1_min .. o0_max .. o1_max
    // => o0_max - o1_min > 0 => intersection


    // normal <- o0 | o1
    // o0_max .. o1_max ... o0_min .. o1_min
    return max0 - min1;
}

/*  From: https://docs.godotengine.org/en/stable/tutorials/math/vectors_advanced.html#collision-detection-in-3d
 */
static void sat_overlap_unsigned(
    const FixedArray<double, 3>& l,
    const CollisionVertices& vertices0,
    const CollisionVertices& vertices1,
    double& overlap0,
    double& overlap1)
{
    double max0 = -INFINITY;
    double max1 = -INFINITY;
    double min0 = INFINITY;
    double min1 = INFINITY;
    for (const auto& v : vertices0) {
        double s = dot0d(v, l);
        max0 = std::max(max0, s);
        min0 = std::min(min0, s);
    }
    for (const auto& v : vertices1) {
        double s = dot0d(v, l);
        max1 = std::max(max1, s);
        min1 = std::min(min1, s);
    }

    overlap0 = max1 - min0;
    overlap1 = max0 - min1;
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
        CollisionEdges edges0;
        CollisionEdges edges1;
        std::vector<const CollisionTriangleSphere*> relevant_triangles0;
        std::vector<const CollisionTriangleSphere*> relevant_triangles1;
        {
            const std::vector<CollisionTriangleSphere>& triangles0 = mesh0.get_triangles_sphere();
            const std::vector<CollisionTriangleSphere>& triangles1 = mesh1.get_triangles_sphere();
            relevant_triangles0.reserve(triangles0.size());
            relevant_triangles1.reserve(triangles1.size());
            for (const auto& t0 : triangles0) {
                if (mesh1.intersects(t0.bounding_sphere) && mesh1.intersects(t0.plane)) {
                    relevant_triangles0.push_back(&t0);
                }
                vertices0.insert(t0.triangle);
            }
            for (const auto& t1 : triangles1) {
                if (mesh0.intersects(t1.bounding_sphere) && mesh0.intersects(t1.plane)) {
                    relevant_triangles1.push_back(&t1);
                }
                vertices1.insert(t1.triangle);
            }
        }

        for (const auto& t0 : relevant_triangles0) {
            edges0.insert(t0->triangle);
        }
        for (const auto& t1 : relevant_triangles1) {
            edges1.insert(t1->triangle);
        }
        for (const auto& t0 : relevant_triangles0) {
            double sat_overl = sat_overlap_signed(
                t0->plane.normal,
                vertices0,
                vertices1);
            if (sat_overl < best_min_overlap) {
                best_min_overlap = sat_overl;
                best_normal = t0->plane.normal;
            }
        }
        for (const auto& t1 : relevant_triangles1) {
            double sat_overl = sat_overlap_signed(
                t1->plane.normal,
                vertices1,
                vertices0);
            if (sat_overl < best_min_overlap) {
                best_min_overlap = sat_overl;
                best_normal = -t1->plane.normal;
            }
        }
        for (const auto& e0 : edges0) {
            for (const auto& e1 : edges1) {
                auto n = cross(e0, e1);
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
