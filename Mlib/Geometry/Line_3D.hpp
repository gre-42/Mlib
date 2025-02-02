#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

template <class TDir, class TPos, size_t tndim>
class PlaneNd;
template <class TPos, size_t tndim>
class BoundingSphere;
template <class TData, size_t tndim>
class AxisAlignedBoundingBox;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
template <class TPos>
struct ColoredVertex;
template <class TDir, class TPos>
class RaySegment3D;

template <class TPos>
class Line3D {
public:
    template <class TPos2>
    explicit Line3D(const FixedArray<ColoredVertex<TPos2>, 2>& vertices);
    template <class TPos2, class TPosTransform>
    Line3D(
        const FixedArray<ColoredVertex<TPos2>, 2>& vertices,
        const TransformationMatrix<float, TPosTransform, 3>& transformation);
    const FixedArray<TPos, 2, 3>& vertices() const;
    template <class TDir>
    RaySegment3D<TDir, TPos> ray() const;
    BoundingSphere<TPos, 3> bounding_sphere() const;
    AxisAlignedBoundingBox<TPos, 3> aabb() const;
private:
    const FixedArray<TPos, 2, 3> vertices_;
};

}
