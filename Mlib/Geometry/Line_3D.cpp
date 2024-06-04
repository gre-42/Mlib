#include "Line_3D.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Plane_Nd.hpp>
#include <Mlib/Geometry/Ray_Segment_3D.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>

using namespace Mlib;

template <class TPos>
Line3D<TPos>::Line3D(
    const FixedArray<ColoredVertex<TPos>, 2>& vertices)
    : vertices_{
        vertices(0).position,
        vertices(1).position }
{}

template <class TPos>
template <class TPos2>
Line3D<TPos>::Line3D(
    const FixedArray<ColoredVertex<TPos2>, 2>& vertices,
    const TransformationMatrix<float, double, 3>& transformation)
    : vertices_{
        transformation.transform(vertices(0).position.template casted<double>()),
        transformation.transform(vertices(1).position.template casted<double>())}
{}

template <class TPos>
const FixedArray<FixedArray<TPos, 3>, 2>& Line3D<TPos>::vertices() const {
    return vertices_;
}

template <class TPos>
RaySegment3D<TPos> Line3D<TPos>::ray() const {
    return RaySegment3D<TPos>{vertices_};
}

template <class TPos>
BoundingSphere<TPos, 3> Line3D<TPos>::bounding_sphere() const
{
    return BoundingSphere<TPos, 3>{vertices_};
}

template <class TPos>
AxisAlignedBoundingBox<TPos, 3> Line3D<TPos>::aabb() const {
    return AxisAlignedBoundingBox<TPos, 3>::from_points(vertices_);
}

namespace Mlib {

template class Line3D<float>;
template class Line3D<double>;

template Line3D<double>::Line3D(const FixedArray<ColoredVertex<float>, 2>&vertices, const TransformationMatrix<float, double, 3>&transformation);
template Line3D<double>::Line3D(const FixedArray<ColoredVertex<double>, 2>&vertices, const TransformationMatrix<float, double, 3>&transformation);

}
