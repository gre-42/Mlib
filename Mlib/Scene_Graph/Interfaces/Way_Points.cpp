#include "Way_Points.hpp"
#include <Mlib/Iterator/Enumerate.hpp>

using namespace Mlib;

WayPointsAndBvh::WayPointsAndBvh(PointsAndAdjacencyResource way_points)
    : way_points{ std::move(way_points) }
    , bvh{ fixed_full<CompressedScenePos, 3>((CompressedScenePos)10.f), 12 }
{
    for (const auto& [i, p] : enumerate(way_points.points)) {
        bvh.insert(AxisAlignedBoundingBox<CompressedScenePos, 3>::from_point(p.position), i);
    }
}
