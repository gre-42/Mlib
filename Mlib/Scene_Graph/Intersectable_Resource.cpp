#include "Intersectable_Resource.hpp"
#include <Mlib/Geometry/Interfaces/IIntersectable.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Typed_Mesh.hpp>

using namespace Mlib;

IntersectableResource::IntersectableResource(
    std::list<TypedMesh<std::shared_ptr<IIntersectable<float>>>>&& intersectables)
    : intersectables_{ std::move(intersectables) }
{}

std::shared_ptr<AnimatedColoredVertexArrays> IntersectableResource::get_physics_arrays() const {
    return std::make_shared<AnimatedColoredVertexArrays>();
}

std::list<TypedMesh<std::shared_ptr<IIntersectable<float>>>> IntersectableResource::get_intersectables() const {
    return intersectables_;
}