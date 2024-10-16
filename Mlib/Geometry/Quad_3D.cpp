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
template <class TPos2, class TPosTransform>
Quad3D<TPos>::Quad3D(
    const FixedArray<ColoredVertex<TPos2>, 4>& vertices,
    const TransformationMatrix<float, TPosTransform, 3>& transformation)
    : vertices_{
        transformation.transform(vertices(0).position.template casted<TPosTransform>()),
        transformation.transform(vertices(1).position.template casted<TPosTransform>()),
        transformation.transform(vertices(2).position.template casted<TPosTransform>()),
        transformation.transform(vertices(3).position.template casted<TPosTransform>()) }
{}

template <class TPos>
const FixedArray<TPos, 4, 3>& Quad3D<TPos>::vertices() const {
    return vertices_;
}

template <class TPos>
ConvexPolygon3D<TPos, 4> Quad3D<TPos>::polygon() const {
    return { vertices_ };
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

template Quad3D<double>::Quad3D(
    const FixedArray<ColoredVertex<float>, 4>& vertices,
    const TransformationMatrix<float, double, 3>& transformation);
template Quad3D<double>::Quad3D(
    const FixedArray<ColoredVertex<double>, 4>& vertices,
    const TransformationMatrix<float, double, 3>& transformation);
template Quad3D<float>::Quad3D(
    const FixedArray<ColoredVertex<float>, 4>& vertices,
    const TransformationMatrix<float, float, 3>& transformation);

}
