#include "Sat_Overlap_Combiner.hpp"
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Geometry/Intersection/Collision_Ridge.hpp>
#include <Mlib/Geometry/Mesh/Sat_Overlap.hpp>

using namespace Mlib;

SatOverlapCombiner::SatOverlapCombiner(
    const std::set<OrderableFixedArray<double, 3>>& vertices0,
    const std::set<OrderableFixedArray<double, 3>>& vertices1)
	: keep_normal_{ false }
	, best_min_overlap_{ (double)INFINITY }
    , vertices0_{ vertices0 }
    , vertices1_{ vertices1 }
{}

double SatOverlapCombiner::overlap_signed(const FixedArray<double, 3>& normal) const {
    return sat_overlap_signed(
        normal,
        vertices0_,
        vertices1_);
}

void SatOverlapCombiner::overlap_unsigned(
    const FixedArray<double, 3>& normal,
    double& overlap0,
    double& overlap1) const
{
    sat_overlap_unsigned(
        normal,
        vertices0_,
        vertices1_,
        overlap0,
        overlap1);
}

void SatOverlapCombiner::combine_sticky_ridge(const CollisionRidgeSphere& e1, double max_keep_normal)
{
    if (max_keep_normal != -INFINITY) {
        double sat_overl = overlap_signed(-e1.normal);
        if (sat_overl < best_min_overlap_) {
            best_min_overlap_ = sat_overl;
            best_normal_ = -e1.normal;
        }
        keep_normal_ = (sat_overl < max_keep_normal);
    }
}

void SatOverlapCombiner::combine_ridges(const CollisionRidgeSphere& e0, const CollisionRidgeSphere& e1) {
    auto n = cross(e0.edge(1) - e0.edge(0), e1.edge(1) - e1.edge(0));
    double l2 = sum(squared(n));
    if (l2 < 1e-6) {
        return;
    }
    n /= std::sqrt(l2);
    double overlap0;
    double overlap1;
    overlap_unsigned(n, overlap0, overlap1);
    if (overlap0 < overlap1) {
        if (e0.is_oriented() && (-dot0d(n, e0.normal) < e0.min_cos - 1e-4)) {
            return;
        }
        if (e1.is_oriented() && (dot0d(n, e1.normal) < e1.min_cos - 1e-4)) {
            return;
        }
        if (overlap0 < best_min_overlap_) {
            best_min_overlap_ = overlap0;
            if (!keep_normal_) {
                best_normal_ = -n;
            }
        }
    } else {
        if (e0.is_oriented() && (dot0d(n, e0.normal) < e0.min_cos - 1e-4)) {
            return;
        }
        if (e1.is_oriented() && (-dot0d(n, e1.normal) < e1.min_cos - 1e-4)) {
            return;
        }
        if (overlap1 < best_min_overlap_) {
            best_min_overlap_ = overlap1;
            if (!keep_normal_) {
                best_normal_ = n;
            }
        }
    }
}

void SatOverlapCombiner::combine_plane(const FixedArray<double, 3>& normal) {
    double sat_overl = overlap_signed(normal);
    if (sat_overl < best_min_overlap_) {
        best_min_overlap_ = sat_overl;
        if (!keep_normal_) {
            best_normal_ = normal;
        }
    }
}
