#include "Set_Bounds.hpp"
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>

using namespace Mlib;

void Mlib::set_bounds(
    AnimatedColoredVertexArrays& dest,
    const AxisAlignedBoundingBox<CompressedScenePos, 3>& aabb,
    const BoundingSphere<CompressedScenePos, 3>& bounding_sphere)
{
    for (auto& cva : dest.scvas) {
        cva->set_bounds(aabb.casted<float>(), bounding_sphere.casted<float>());
    }
    for (auto& cva : dest.dcvas) {
        cva->set_bounds(aabb, bounding_sphere);
    }
}
