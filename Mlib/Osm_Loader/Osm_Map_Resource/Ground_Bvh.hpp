#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <memory>

namespace Mlib {

template <class TPos>
struct ColoredVertex;
template <class TPos>
class TriangleList;
template <class TPos>
class ColoredVertexArray;

class GroundBvh {
    using Triangle3d = FixedArray<double, 3, 3>;
    using Triangle2d = FixedArray<double, 3, 2>;
public:
    explicit GroundBvh(const std::list<std::shared_ptr<TriangleList<double>>>& triangles);
    explicit GroundBvh(const std::list<std::shared_ptr<ColoredVertexArray<double>>>& cvas);
    bool height(double& height, const FixedArray<double, 2>& pt) const;
    bool height3d(double& height, const FixedArray<double, 3>& pt) const;
    bool gradient(FixedArray<double, 2>& grad, const FixedArray<double, 2>& pt, double dx) const;
    void print(std::ostream& ostr, const BvhPrintingOptions& opts, size_t rec = 0) const;
private:
    void maybe_add_triangle(const FixedArray<ColoredVertex<double>, 3>& t);

    GroundBvh();
    Bvh<double, 2, Triangle3d> bvh_;
};

}
