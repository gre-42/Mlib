#pragma once
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Intersection/Intersect_Lines.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Scene_Precision.hpp>

namespace Mlib {

template <class TVisitor>
bool visit_contour_intersections(
    const std::vector<std::vector<FixedArray<CompressedScenePos, 2>>>& contours,
    const TVisitor& visitor)
{
    using Edge = FixedArray<CompressedScenePos, 2, 2>;
    Bvh<CompressedScenePos, 2, Edge> bvh{{(CompressedScenePos)100., (CompressedScenePos)100.}, 10};
    for (const auto& c : contours) {
        for (auto it = c.begin(); it != c.end(); ++it) {
            auto s = it;
            ++s;
            auto edge = Edge{
                *it,
                (s == c.end()) ? c.front() : *s};
            auto aabb = AxisAlignedBoundingBox<CompressedScenePos, 2>::from_points(edge);
            if (!bvh.visit(
                aabb,
                [&edge, &visitor](const Edge& data)
            {
                FixedArray<ScenePos, 2> intersection = uninitialized;
                if (intersect_lines(intersection, funpack(edge), funpack(data), 0., 0., false, true)) {
                    return visitor(intersection);
                }
                return true;
            }))
            {
                return false;
            }
            bvh.insert(aabb, edge);
        }
    }
    return true;
}

}
