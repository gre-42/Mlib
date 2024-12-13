#include "Line_3D.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Plane_Nd.hpp>
#include <Mlib/Geometry/Ray_Segment_3D.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Scene_Precision.hpp>

using namespace Mlib;

template <class TPos>
template <class TPos2>
Line3D<TPos>::Line3D(
    const FixedArray<ColoredVertex<TPos2>, 2>& vertices)
    : vertices_{
        vertices(0).position.template casted<TPos>(),
        vertices(1).position.template casted<TPos>() }
{}

template <class TPos>
template <class TPos2, class TPosTransform>
Line3D<TPos>::Line3D(
    const FixedArray<ColoredVertex<TPos2>, 2>& vertices,
    const TransformationMatrix<float, TPosTransform, 3>& transformation)
    : vertices_{
        transformation.transform(vertices(0).position.template casted<TPosTransform>()),
        transformation.transform(vertices(1).position.template casted<TPosTransform>())}
{}

template <class TPos>
const FixedArray<TPos, 2, 3>& Line3D<TPos>::vertices() const {
    return vertices_;
}

template <class TPos>
template <class TDir>
RaySegment3D<TDir, TPos> Line3D<TPos>::ray() const {
    return RaySegment3D<TDir, TPos>{vertices_};
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
template class Line3D<ScenePos>;

template Line3D<float>::Line3D(const FixedArray<ColoredVertex<float>, 2>& vertices);
template Line3D<ScenePos>::Line3D(const FixedArray<ColoredVertex<float>, 2>& vertices);
template Line3D<ScenePos>::Line3D(const FixedArray<ColoredVertex<CompressedScenePos>, 2>& vertices);

template Line3D<ScenePos>::Line3D(
    const FixedArray<ColoredVertex<float>, 2>& vertices,
    const TransformationMatrix<float, ScenePos, 3>& transformation);
template Line3D<ScenePos>::Line3D(
    const FixedArray<ColoredVertex<CompressedScenePos>, 2>& vertices,
    const TransformationMatrix<float, ScenePos, 3>& transformation);
template Line3D<float>::Line3D(
    const FixedArray<ColoredVertex<float>, 2>& vertices,
    const TransformationMatrix<float, float, 3>& transformation);

template RaySegment3D<float, ScenePos> Line3D<ScenePos>::ray() const;

}
