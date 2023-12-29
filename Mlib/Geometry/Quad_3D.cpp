#include "Quad_3D.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Convex_Polygon.hpp>
#include <Mlib/Geometry/Intersection/Welzl.hpp>
#include <Mlib/Geometry/Plane_Nd.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>

using namespace Mlib;

template <class TPos>
Quad3D::Quad3D(
    const FixedArray<ColoredVertex<TPos>, 4>& vertices,
    const TransformationMatrix<float, double, 3>& transformation)
: vertices_{
    transformation.transform(vertices(0).position TEMPLATEV casted<double>()),
    transformation.transform(vertices(1).position TEMPLATEV casted<double>()),
    transformation.transform(vertices(2).position TEMPLATEV casted<double>()),
    transformation.transform(vertices(3).position TEMPLATEV casted<double>()),}
{}

const FixedArray<FixedArray<double, 3>, 4>& Quad3D::vertices() const {
    return vertices_;
}

ConvexPolygon3D<double, 4> Quad3D::polygon() const {
    return ConvexPolygon3D<double, 4>{ vertices_ };
}

BoundingSphere<double, 3> Quad3D::bounding_sphere(std::minstd_rand& rng) const {
    return welzl_from_fixed<double, 3>(vertices_, rng);
}

AxisAlignedBoundingBox<double, 3> Quad3D::aabb() const {
    return AxisAlignedBoundingBox<double, 3>{ vertices_ };
}


template Quad3D::Quad3D(const FixedArray<ColoredVertex<float>, 4>& vertices, const TransformationMatrix<float, double, 3>& transformation);
template Quad3D::Quad3D(const FixedArray<ColoredVertex<double>, 4>& vertices, const TransformationMatrix<float, double, 3>& transformation);
