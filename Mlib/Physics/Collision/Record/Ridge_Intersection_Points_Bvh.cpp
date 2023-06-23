#include "Ridge_Intersection_Points_Bvh.hpp"
#include <Mlib/Physics/Physics_Engine/Physics_Engine_Config.hpp>

using namespace Mlib;

RidgeIntersectionPointsBvh::RidgeIntersectionPointsBvh(const PhysicsEngineConfig& cfg)
: bvh_{{cfg.bvh_max_size, cfg.bvh_max_size, cfg.bvh_max_size}, cfg.bvh_levels},
  radius_{cfg.intersection_point_radius}
{}

void RidgeIntersectionPointsBvh::insert(const FixedArray<double, 3>& intersection_point) {
    bvh_.insert(AxisAlignedBoundingBox{intersection_point}, intersection_point);
}

bool RidgeIntersectionPointsBvh::has_neighbor(const FixedArray<double, 3>& intersection_point)
{
    return bvh_.has_neighbor2(
        intersection_point,
        radius_,
        [&intersection_point](const FixedArray<double, 3>& candidate){
            return sum(squared(intersection_point - candidate));
        });
}
