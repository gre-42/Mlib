#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <cstdint>
#include <memory>

namespace Mlib {

template <class TPos>
struct ColoredVertex;
template <class TPos>
class TriangleList;
template <class TPos>
class ColoredVertexArray;
enum class PhysicsMaterial: uint32_t;
template <class T>
struct Interval;

using GroundTriangle3d = FixedArray<CompressedScenePos, 3, 3>;
using GroundTriangle2d = FixedArray<CompressedScenePos, 3, 2>;

struct GroundTriangle3dAndMaterial {
    GroundTriangle3d triangle;
    PhysicsMaterial physics_material;
    bool height(
        CompressedScenePos& height,
        const FixedArray<CompressedScenePos, 2>& pt) const;
};

class GroundBvh {
public:
    static constexpr CompressedScenePos MIN_BRIDGE_GROUND = std::numeric_limits<CompressedScenePos>::lowest();
    static constexpr CompressedScenePos MAX_BRIDGE_AIR = std::numeric_limits<CompressedScenePos>::max();
    
    explicit GroundBvh(const std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>& triangles);
    explicit GroundBvh(const std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& cvas);
    ~GroundBvh();
    bool max_height(CompressedScenePos& height, const FixedArray<CompressedScenePos, 2>& pt) const;
    bool height3d(CompressedScenePos& height, const FixedArray<CompressedScenePos, 3>& pt) const;
    bool gradient(
        FixedArray<double, 2>& grad,
        const FixedArray<CompressedScenePos, 2>& pt,
        CompressedScenePos dx) const;
    Interval<CompressedScenePos> bridge_gap(
        const FixedArray<CompressedScenePos, 2>& pt) const;
    void print(std::ostream& ostr, const BvhPrintingOptions& opts, size_t rec = 0) const;
private:
    void maybe_add_triangle(
        const FixedArray<ColoredVertex<CompressedScenePos>, 3>& t,
        PhysicsMaterial physics_material);
    GroundBvh();
    Bvh<CompressedScenePos, 2, GroundTriangle3dAndMaterial> bvh_;
};

}
