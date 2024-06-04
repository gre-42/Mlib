#include "Quad_3D.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Convex_Polygon.hpp>
#include <Mlib/Geometry/Intersection/Welzl.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>

using namespace Mlib;

template <class TPos>
Quad3D<TPos>::Quad3D(const FixedArray<ColoredVertex<TPos>, 4>& vertices)
    : vertices_{
        vertices(0).position,
        vertices(1).position,
        vertices(2).position,
        vertices(3).position }
{}

template <class TPos>
Quad3D<TPos>::Quad3D(
    const FixedArray<ColoredVertex<TPos>, 4>& vertices,
    const TransformationMatrix<float, TPos, 3>& transformation)
    : vertices_{
        transformation.transform(vertices(0).position),
        transformation.transform(vertices(1).position),
        transformation.transform(vertices(2).position),
        transformation.transform(vertices(3).position) }
{}

template <class TPos>
const FixedArray<FixedArray<TPos, 3>, 4>& Quad3D<TPos>::vertices() const {
    return vertices_;
}

template <class TPos>
ConvexPolygon3D<TPos, 4> Quad3D<TPos>::polygon() const {
    return ConvexPolygon3D<TPos, 4>{ vertices_ };
}

template <class TPos>
BoundingSphere<TPos, 3> Quad3D<TPos>::bounding_sphere(std::minstd_rand& rng) const {
    return welzl_from_fixed<TPos, 3>(vertices_, rng);
}

template <class TPos>
AxisAlignedBoundingBox<TPos, 3> Quad3D<TPos>::aabb() const {
    return AxisAlignedBoundingBox<TPos, 3>::from_points(vertices_);
}

namespace Mlib {

template class Quad3D<float>;
template class Quad3D<double>;

}
