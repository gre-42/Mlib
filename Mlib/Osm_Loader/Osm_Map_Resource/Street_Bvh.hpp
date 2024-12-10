#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <list>

namespace Mlib {

template <class TPos>
struct ColoredVertex;

class StreetBvh {
    typedef FixedArray<double, 3, 2> Triangle2d;
public:
    explicit StreetBvh(const std::list<FixedArray<ColoredVertex<double>, 3>>& triangles);
    double min_dist(
        const FixedArray<double, 2>& pt,
        double max_dist,
        FixedArray<double, 2>* closest_pt = nullptr) const;
    bool has_neighbor(const FixedArray<double, 2>& pt, double max_dist) const;
private:
    Bvh<double, 2, Triangle2d> bvh_;
};

}
