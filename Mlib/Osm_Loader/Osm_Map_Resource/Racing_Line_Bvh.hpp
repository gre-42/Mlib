#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>

namespace Mlib {

class RacingLineBvh {
    typedef FixedArray<FixedArray<float, 2>, 2> Line2d;
public:
    explicit RacingLineBvh();
    void insert(const Line2d& racing_line_segment);
    float intersecting_way_beta(const Line2d& way_boundary) const;
private:
    Bvh<float, Line2d, 2> bvh_;
};

}
