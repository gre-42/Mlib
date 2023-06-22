#include "Line3D.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Plane_Nd.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>

using namespace Mlib;

template <class TPos>
Line3D::Line3D(
    const FixedArray<ColoredVertex<TPos>, 2>& vertices,
    const TransformationMatrix<float, double, 3>& transformation)
: vertices_{
    transformation.transform(vertices(0).position TEMPLATEV casted<double>()),
    transformation.transform(vertices(1).position TEMPLATEV casted<double>())}
{}

const FixedArray<FixedArray<double, 3>, 2>& Line3D::vertices() const {
    return vertices_;
}

BoundingSphere<double, 3> Line3D::bounding_sphere() const
{
    return BoundingSphere<double, 3>{vertices_};
}

AxisAlignedBoundingBox<double, 3> Line3D::aabb() const {
    return AxisAlignedBoundingBox<double, 3>{vertices_};
}

template Line3D::Line3D(const FixedArray<ColoredVertex<float>, 2>& vertices, const TransformationMatrix<float, double, 3>& transformation);
template Line3D::Line3D(const FixedArray<ColoredVertex<double>, 2>& vertices, const TransformationMatrix<float, double, 3>& transformation);
