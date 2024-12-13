#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <memory>

namespace Mlib {

template <class TPos>
struct ColoredVertex;
template <class TPos>
class TriangleList;
template <class TPos>
class ColoredVertexArray;

class GroundBvh {
    using Triangle3d = FixedArray<CompressedScenePos, 3, 3>;
    using Triangle2d = FixedArray<CompressedScenePos, 3, 2>;
public:
    explicit GroundBvh(const std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>& triangles);
    explicit GroundBvh(const std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& cvas);
    bool height(CompressedScenePos& height, const FixedArray<CompressedScenePos, 2>& pt) const;
    bool height3d(CompressedScenePos& height, const FixedArray<CompressedScenePos, 3>& pt) const;
    bool gradient(FixedArray<double, 2>& grad, const FixedArray<CompressedScenePos, 2>& pt, CompressedScenePos dx) const;
    void print(std::ostream& ostr, const BvhPrintingOptions& opts, size_t rec = 0) const;
private:
    void maybe_add_triangle(const FixedArray<ColoredVertex<CompressedScenePos>, 3>& t);

    GroundBvh();
    Bvh<CompressedScenePos, 2, Triangle3d> bvh_;
};

}
