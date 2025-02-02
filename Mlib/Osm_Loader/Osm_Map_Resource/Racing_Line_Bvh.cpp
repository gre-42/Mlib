#include "Racing_Line_Bvh.hpp"
#include <Mlib/Geometry/Intersection/Distance_Point_Line.hpp>
#include <Mlib/Geometry/Intersection/Intersect_Lines.hpp>

using namespace Mlib;

RacingLineBvh::RacingLineBvh()
    : bvh_{ {(CompressedScenePos)100.f, (CompressedScenePos)100.f}, 10 }
{}

void RacingLineBvh::insert(const RacingLineSegment& racing_line_segment) {
    bvh_.insert(
        AxisAlignedBoundingBox<CompressedScenePos, 2>::from_points(racing_line_segment.racing_line_segment),
        racing_line_segment);
}

void RacingLineBvh::intersecting_way_beta(
    const RacingLineSegment::Line2d& way_boundary,
    double& beta,
    const RacingLineSegment** racing_line_segment) const
{
    beta = NAN;
    bvh_.visit(
        AxisAlignedBoundingBox<CompressedScenePos, 2>::from_points(way_boundary),
        [&way_boundary, &beta, &racing_line_segment](const RacingLineSegment& candidate_racing_line_segment)
    {
        FixedArray<double, 2> intersection_point = uninitialized;
        if (intersect_lines(
            intersection_point,
            candidate_racing_line_segment.racing_line_segment.casted<double>(),
            way_boundary.casted<double>(),
            0.,     // width0
            0.,     // width1
            false,  // compute_center
            true))  // check_bounds
        {
            if (!std::isnan(beta)) {
                lerr() << "WARNING: Found multiple racing line segments for way connection";
                beta = NAN;
                return false;
            }
            beta = transform_to_line_coordinates(
                intersection_point,
                way_boundary[0].casted<double>(),
                way_boundary[1].casted<double>(),
                true)(0);  // compute_center
            *racing_line_segment = &candidate_racing_line_segment;
        }
        return true;
    });
}
