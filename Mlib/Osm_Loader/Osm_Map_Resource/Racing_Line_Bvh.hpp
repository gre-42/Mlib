#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>

namespace Mlib {

struct RacingLineSegment {
    typedef FixedArray<FixedArray<float, 2>, 2> Line2d;
    Line2d racing_line_segment;
    FixedArray<float, 3> color;
};

class RacingLineBvh {
public:
    explicit RacingLineBvh();
    void insert(const RacingLineSegment& racing_line_segment);
    void intersecting_way_beta(
        const RacingLineSegment::Line2d& way_boundary,
        float& beta,
        const RacingLineSegment** racing_line_segment) const;
private:
    Bvh<float, RacingLineSegment, 2> bvh_;
};

}
