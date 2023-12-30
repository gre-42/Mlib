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

class Line3D {
public:
    template <class TPos>
    Line3D(
        const FixedArray<ColoredVertex<TPos>, 2>& vertices,
        const TransformationMatrix<float, double, 3>& transformation);
    const FixedArray<FixedArray<double, 3>, 2>& vertices() const;
    RaySegment3D<double> ray() const;
    BoundingSphere<double, 3> bounding_sphere() const;
    AxisAlignedBoundingBox<double, 3> aabb() const;
private:
    const FixedArray<FixedArray<double, 3>, 2> vertices_;
};

}
