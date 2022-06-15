#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <memory>

namespace Mlib {

template <class TPos>
struct ColoredVertex;
template <class TPos>
class TriangleList;
template <class TPos>
struct ColoredVertexArray;

class GroundBvh {
    using Triangle3d = FixedArray<FixedArray<double, 3>, 3>;
    using Triangle2d = FixedArray<FixedArray<double, 2>, 3>;
public:
    explicit GroundBvh(const std::list<std::shared_ptr<TriangleList<double>>>& triangles);
    explicit GroundBvh(const std::list<std::shared_ptr<ColoredVertexArray<double>>>& cvas);
    bool height(double& height, const FixedArray<double, 2>& pt) const;
    bool height3d(double& height, const FixedArray<double, 3>& pt) const;
    void print(std::ostream& ostr, const BvhPrintingOptions& opts, size_t rec = 0) const;
private:
    GroundBvh();
    Bvh<double, Triangle3d, 2> bvh_;
};

}
