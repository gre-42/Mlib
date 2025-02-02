#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <random>

namespace Mlib {

template <class TDir, class TPos, size_t tnvertices>
class ConvexPolygon3D;
template <class TPos, size_t tndim>
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
    using I = funpack_t<TPos>;
    explicit Quad3D(const FixedArray<TPos, 4, 3>& vertices);
    template <class TPos2>
    explicit Quad3D(const FixedArray<ColoredVertex<TPos2>, 4>& vertices);
    template <class TPos2, class TPosTransform>
    Quad3D(
        const FixedArray<ColoredVertex<TPos2>, 4>& vertices,
        const TransformationMatrix<float, TPosTransform, 3>& transformation);
    const FixedArray<TPos, 4, 3>& vertices() const;
    ConvexPolygon3D<I, TPos, 4> polygon() const;
    BoundingSphere<TPos, 3> bounding_sphere(std::minstd_rand& rng) const;
    AxisAlignedBoundingBox<TPos, 3> aabb() const;
private:
    const FixedArray<TPos, 4, 3> vertices_;
};

}
