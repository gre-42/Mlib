#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <list>

namespace Mlib {

template <class TPos>
struct ColoredVertex;

class StreetBvh {
    typedef FixedArray<CompressedScenePos, 3, 2> Triangle2d;
public:
    explicit StreetBvh(const std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>>& triangles);
    std::optional<CompressedScenePos> min_dist(
        const FixedArray<CompressedScenePos, 2>& pt,
        CompressedScenePos max_dist,
        FixedArray<CompressedScenePos, 2>* closest_pt = nullptr) const;
    bool has_neighbor(const FixedArray<CompressedScenePos, 2>& pt, CompressedScenePos max_dist) const;
private:
    Bvh<CompressedScenePos, 2, Triangle2d> bvh_;
};

}
