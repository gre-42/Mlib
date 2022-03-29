#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>

namespace Mlib {

struct ColoredVertex;

class StreetBvh {
    typedef FixedArray<FixedArray<float, 2>, 3> Triangle2d;
public:
    explicit StreetBvh(const std::list<FixedArray<ColoredVertex, 3>>& triangles);
    float min_dist(const FixedArray<float, 2>& pt, float max_dist) const;
    bool has_neighbor(const FixedArray<float, 2>& pt, float max_dist) const;
private:
    Bvh<float, Triangle2d, 2> bvh_;
};

}
