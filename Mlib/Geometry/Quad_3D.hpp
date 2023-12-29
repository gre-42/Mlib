#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <random>

namespace Mlib {

template <class TData, size_t tnvertices>
class ConvexPolygon3D;
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

class Quad3D {
public:
    template <class TPos>
    Quad3D(
        const FixedArray<ColoredVertex<TPos>, 4>& vertices,
        const TransformationMatrix<float, double, 3>& transformation);
    const FixedArray<FixedArray<double, 3>, 4>& vertices() const;
    ConvexPolygon3D<double, 4> polygon() const;
    BoundingSphere<double, 3> bounding_sphere(std::minstd_rand& rng) const;
    AxisAlignedBoundingBox<double, 3> aabb() const;
private:
    const FixedArray<FixedArray<double, 3>, 4> vertices_;
};

}
