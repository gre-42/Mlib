#include "Ridge_Intersection_Points_Bvh.hpp"
#include <Mlib/Physics/Physics_Engine/Physics_Engine_Config.hpp>

using namespace Mlib;

RidgeIntersectionPointsBvh::RidgeIntersectionPointsBvh(const PhysicsEngineConfig& cfg)
    : bvh_{{cfg.bvh_max_size, cfg.bvh_max_size, cfg.bvh_max_size}, cfg.bvh_levels}
    , radius_{cfg.intersection_point_radius}
{}

void RidgeIntersectionPointsBvh::insert(const FixedArray<ScenePos, 3>& intersection_point) {
    auto ip = intersection_point.casted<CompressedScenePos>();
    bvh_.insert(AxisAlignedBoundingBox<CompressedScenePos, 3>::from_point(ip), ip);
}

bool RidgeIntersectionPointsBvh::has_neighbor(const FixedArray<ScenePos, 3>& intersection_point)
{
    auto ip = intersection_point.casted<CompressedScenePos>();
    return bvh_.has_neighbor2(
        ip,
        (CompressedScenePos)radius_,
        [&ip](const FixedArray<CompressedScenePos, 3>& candidate){
            return sum(squared(ip - candidate));
        });
}
