#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <memory>

namespace Mlib {

struct ColoredVertex;
class TriangleList;
class ColoredVertexArray;

class GroundBvh {
    using Triangle3d = FixedArray<FixedArray<float, 3>, 3>;
    using Triangle2d = FixedArray<FixedArray<float, 2>, 3>;
public:
    explicit GroundBvh(const std::list<std::shared_ptr<TriangleList>>& triangles);
    explicit GroundBvh(const std::list<std::shared_ptr<ColoredVertexArray>>& cvas);
    bool height(float& height, const FixedArray<float, 2>& pt) const;
    void print(std::ostream& ostr, const BvhPrintingOptions& opts, size_t rec = 0) const;
private:
    GroundBvh();
    Bvh<float, Triangle3d, 2> bvh_;
};

}
