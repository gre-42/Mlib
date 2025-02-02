#include "Triangle_3D.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Convex_Polygon.hpp>
#include <Mlib/Geometry/Intersection/Welzl.hpp>
#include <Mlib/Geometry/Plane_Nd.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Scene_Precision.hpp>

using namespace Mlib;

template <class TPos>
Triangle3D<TPos>::Triangle3D(const FixedArray<TPos, 3, 3>& vertices)
    : vertices_{ vertices }
{}

template <class TPos>
template <class TPos2>
Triangle3D<TPos>::Triangle3D(const FixedArray<ColoredVertex<TPos2>, 3>& vertices)
    : vertices_{
        vertices(0).position.template casted<TPos>(),
        vertices(1).position.template casted<TPos>(),
        vertices(2).position.template casted<TPos>()}
{}

template <class TPos>
template <class TPos2, class TPosTransform>
Triangle3D<TPos>::Triangle3D(
    const FixedArray<ColoredVertex<TPos2>, 3>& vertices,
    const TransformationMatrix<float, TPosTransform, 3>& transformation)
    : vertices_{
        transformation.transform(vertices(0).position.template casted<TPosTransform>()).template casted<TPos>(),
        transformation.transform(vertices(1).position.template casted<TPosTransform>()).template casted<TPos>(),
        transformation.transform(vertices(2).position.template casted<TPosTransform>()).template casted<TPos>()}
{}

template <class TPos>
const FixedArray<TPos, 3, 3>& Triangle3D<TPos>::vertices() const {
    return vertices_;
}

template <class TPos>
ConvexPolygon3D<typename Triangle3D<TPos>::I, TPos, 3> Triangle3D<TPos>::polygon() const {
    return ConvexPolygon3D<typename Triangle3D<TPos>::I, TPos, 3>{ vertices_ };
}

template <class TPos>
BoundingSphere<TPos, 3> Triangle3D<TPos>::bounding_sphere(std::minstd_rand& rng) const {
    return welzl_from_fixed(funpack(vertices_), rng).template casted<TPos>();
}

template <class TPos>
AxisAlignedBoundingBox<TPos, 3> Triangle3D<TPos>::aabb() const {
    return AxisAlignedBoundingBox<TPos, 3>::from_points(vertices_);
}

namespace Mlib {

template class Triangle3D<float>;
template class Triangle3D<CompressedScenePos>;

template Triangle3D<CompressedScenePos>::Triangle3D(
    const FixedArray<ColoredVertex<float>, 3>& vertices);
template Triangle3D<CompressedScenePos>::Triangle3D(
    const FixedArray<ColoredVertex<CompressedScenePos>, 3>& vertices);
template Triangle3D<CompressedScenePos>::Triangle3D(
    const FixedArray<ColoredVertex<float>, 3>& vertices,
    const TransformationMatrix<float, ScenePos, 3>& transformation);
template Triangle3D<CompressedScenePos>::Triangle3D(
    const FixedArray<ColoredVertex<CompressedScenePos>, 3>& vertices,
    const TransformationMatrix<float, ScenePos, 3>& transformation);
template Triangle3D<float>::Triangle3D(
    const FixedArray<ColoredVertex<float>, 3>& vertices,
    const TransformationMatrix<float, float, 3>& transformation);

}
