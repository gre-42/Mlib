#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>

namespace Mlib {

struct AnimatedColoredVertexArrays;
template <class TData, size_t tndim>
class AxisAlignedBoundingBox;
template <class TPos, size_t tndim>
class BoundingSphere;

void set_bounds(
    AnimatedColoredVertexArrays& dest,
    const AxisAlignedBoundingBox<CompressedScenePos, 3>& aabb,
    const BoundingSphere<CompressedScenePos, 3>& bounding_sphere);

}
