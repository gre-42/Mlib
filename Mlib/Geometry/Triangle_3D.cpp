#include "Triangle_3D.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Convex_Polygon.hpp>
#include <Mlib/Geometry/Intersection/Welzl.hpp>
#include <Mlib/Geometry/Plane_Nd.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>

using namespace Mlib;

template <class TPos>
Triangle3D::Triangle3D(
    const FixedArray<ColoredVertex<TPos>, 3>& vertices,
    const TransformationMatrix<float, double, 3>& transformation)
: vertices_{
    transformation.transform(vertices(0).position TEMPLATEV casted<double>()),
    transformation.transform(vertices(1).position TEMPLATEV casted<double>()),
    transformation.transform(vertices(2).position TEMPLATEV casted<double>())}
{}

const FixedArray<FixedArray<double, 3>, 3>& Triangle3D::vertices() const {
    return vertices_;
}

ConvexPolygon3D<double, 3> Triangle3D::polygon() const {
    return ConvexPolygon3D<double, 3>{ vertices_ };
}

BoundingSphere<double, 3> Triangle3D::bounding_sphere(std::minstd_rand& rng) const {
    return circumscribed_sphere<double, 3>(vertices_, rng);
}

AxisAlignedBoundingBox<double, 3> Triangle3D::aabb() const {
    return AxisAlignedBoundingBox<double, 3>{ vertices_ };
}


template Triangle3D::Triangle3D(const FixedArray<ColoredVertex<float>, 3>& vertices, const TransformationMatrix<float, double, 3>& transformation);
template Triangle3D::Triangle3D(const FixedArray<ColoredVertex<double>, 3>& vertices, const TransformationMatrix<float, double, 3>& transformation);
