#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <random>

namespace Mlib {

template <class TData, size_t tnvertices>
class ConvexPolygon3D;
template <class TData, size_t tndim>
class BoundingSphere;
template <class TData, size_t tndim>
class AxisAlignedBoundingBox;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
template <class TPos>
struct ColoredVertex;

template <class TPos>
class Triangle3D {
public:
    explicit Triangle3D(const FixedArray<ColoredVertex<TPos>, 3>& vertices);
    template <class TPos2, class TPosTransform>
    Triangle3D(
        const FixedArray<ColoredVertex<TPos2>, 3>& vertices,
        const TransformationMatrix<float, TPosTransform, 3>& transformation);
    const FixedArray<TPos, 3, 3>& vertices() const;
    ConvexPolygon3D<TPos, 3> polygon() const;
    BoundingSphere<TPos, 3> bounding_sphere(std::minstd_rand& rng) const;
    AxisAlignedBoundingBox<TPos, 3> aabb() const;
private:
    const FixedArray<TPos, 3, 3> vertices_;
};

}
