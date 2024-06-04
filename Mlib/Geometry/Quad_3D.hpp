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
class Quad3D {
public:
    explicit Quad3D(const FixedArray<ColoredVertex<TPos>, 4>& vertices);
    Quad3D(
        const FixedArray<ColoredVertex<TPos>, 4>& vertices,
        const TransformationMatrix<float, TPos, 3>& transformation);
    const FixedArray<FixedArray<TPos, 3>, 4>& vertices() const;
    ConvexPolygon3D<TPos, 4> polygon() const;
    BoundingSphere<TPos, 3> bounding_sphere(std::minstd_rand& rng) const;
    AxisAlignedBoundingBox<TPos, 3> aabb() const;
private:
    const FixedArray<FixedArray<TPos, 3>, 4> vertices_;
};

}
