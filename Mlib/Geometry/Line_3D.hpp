#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

template <class TData, size_t tndim>
class PlaneNd;
template <class TData, size_t tndim>
class BoundingSphere;
template <class TData, size_t tndim>
class AxisAlignedBoundingBox;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
template <class TPos>
struct ColoredVertex;
template <class TData>
class RaySegment3D;

template <class TPos>
class Line3D {
public:
    explicit Line3D(const FixedArray<ColoredVertex<TPos>, 2>& vertices);
    template <class TPos2>
    Line3D(
        const FixedArray<ColoredVertex<TPos2>, 2>& vertices,
        const TransformationMatrix<float, double, 3>& transformation);
    const FixedArray<FixedArray<TPos, 3>, 2>& vertices() const;
    RaySegment3D<TPos> ray() const;
    BoundingSphere<TPos, 3> bounding_sphere() const;
    AxisAlignedBoundingBox<TPos, 3> aabb() const;
private:
    const FixedArray<FixedArray<TPos, 3>, 2> vertices_;
};

}
