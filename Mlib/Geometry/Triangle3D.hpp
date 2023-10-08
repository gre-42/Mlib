#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <random>

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

class Triangle3D {
public:
    template <class TPos>
    Triangle3D(
        const FixedArray<ColoredVertex<TPos>, 3>& vertices,
        const TransformationMatrix<float, double, 3>& transformation);
    const FixedArray<FixedArray<double, 3>, 3>& vertices() const;
    PlaneNd<double, 3> plane() const;
    BoundingSphere<double, 3> bounding_sphere(std::minstd_rand& rng) const;
    AxisAlignedBoundingBox<double, 3> aabb() const;
private:
    const FixedArray<FixedArray<double, 3>, 3> vertices_;
};

}
