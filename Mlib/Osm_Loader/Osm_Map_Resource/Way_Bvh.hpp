#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>

namespace Mlib {

class WayBvh {
    typedef FixedArray<FixedArray<double, 2>, 2> Line2d;
public:
    explicit WayBvh(const std::list<Line2d>& way_segments);
    void nearest_way(
        const FixedArray<double, 2>& pt,
        double max_dist,
        FixedArray<double, 2>& dir,
        double& distance) const;
private:
    Bvh<double, Line2d, 2> bvh_;
};

}
