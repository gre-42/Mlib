#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>

namespace Mlib {

template <class TPos>
struct ColoredVertex;

class StreetBvh {
    typedef FixedArray<FixedArray<double, 2>, 3> Triangle2d;
public:
    explicit StreetBvh(const std::list<FixedArray<ColoredVertex<double>, 3>>& triangles);
    double min_dist(const FixedArray<double, 2>& pt, double max_dist) const;
    bool has_neighbor(const FixedArray<double, 2>& pt, double max_dist) const;
private:
    Bvh<double, Triangle2d, 2> bvh_;
};

}
