#pragma once
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Intersection/Distance/Distance_Point_Line.hpp>
#include <Mlib/Geometry/Intersection/Intersect_Lines.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Scene_Precision.hpp>

namespace Mlib {

class EdgeIntersectionVisitor {
public:
    using Edge = FixedArray<CompressedScenePos, 2, 2>;
    struct EdgeAndIndex {
        Edge edge;
        size_t index;
    };
    EdgeIntersectionVisitor()
        : bvh_{ {(CompressedScenePos)100., (CompressedScenePos)100.}, 10 }
    {}
    template <class TVisitor>
    bool add(const Edge& edge, size_t index, const TVisitor& visitor) {
        auto aabb = AxisAlignedBoundingBox<CompressedScenePos, 2>::from_points(edge);
        if (!bvh_.visit(
            aabb,
            [index, &edge, &visitor](const EdgeAndIndex& data)
        {
            {
                FixedArray<ScenePos, 2> intersection = uninitialized;
                if (intersect_lines(intersection, funpack(edge), funpack(data.edge), 0., 0., false, true)) {
                    return visitor(intersection, index, data.index, 0.);
                }
            }
            return true;
        }))
        {
            return false;
        }
        bvh_.insert(aabb, EdgeAndIndex{ edge, index });
        return true;
    }
    template <class TVisitor>
    bool visit_close_points(const TVisitor& visitor) {
        return bvh_.visit_all([this, &visitor](const auto& item0){
            const EdgeAndIndex& data0 = item0.payload();
            auto aabb = AxisAlignedBoundingBox<CompressedScenePos, 2>::from_points(data0.edge);
            return bvh_.visit(
                aabb,
                [&data0, &visitor](const EdgeAndIndex& data1)
            {
                for (const auto& p : data0.edge.row_iterable()) {
                    if (any(p != data1.edge[0]) &&
                        any(p != data1.edge[1]))
                    {
                        FixedArray<ScenePos, 2> dir = uninitialized;
                        ScenePos distance;
                        distance_point_to_line(
                            funpack(p),
                            funpack(data1.edge[0]),
                            funpack(data1.edge[1]),
                            dir,
                            distance);
                        if (!visitor(funpack(p), data0.index, data1.index, distance)) {
                            return false;
                        }
                    }
                }
                return true;
            });
        });
    }
private:
    Bvh<CompressedScenePos, 2, EdgeAndIndex> bvh_;
};

template <class TVisitor>
bool visit_contour_intersections(
    const std::vector<std::vector<FixedArray<CompressedScenePos, 2>>>& contours,
    const TVisitor& visitor)
{
    EdgeIntersectionVisitor eiv;
    for (const auto& [i, c] : enumerate(contours)) {
        for (auto it = c.begin(); it != c.end(); ++it) {
            auto s = it;
            ++s;
            auto edge = EdgeIntersectionVisitor::Edge{
                *it,
                (s == c.end()) ? c.front() : *s};
            if (!eiv.add(edge, i, visitor)) {
                return false;
            }
        }
    }
    return eiv.visit_close_points(visitor);
}

}
