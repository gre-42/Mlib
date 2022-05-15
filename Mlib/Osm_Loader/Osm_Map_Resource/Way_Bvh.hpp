#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>

namespace Mlib {

class WayBvh {
    typedef FixedArray<FixedArray<float, 2>, 2> Line2d;
public:
    explicit WayBvh(const std::list<Line2d>& way_segments);
    void nearest_way(
        const FixedArray<float, 2>& pt,
        float max_dist,
        FixedArray<float, 2>& dir,
        float& distance) const;
private:
    Bvh<float, Line2d, 2> bvh_;
};

}
