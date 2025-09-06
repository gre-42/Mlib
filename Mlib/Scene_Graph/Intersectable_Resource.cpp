#include "Intersectable_Resource.hpp"
#include <Mlib/Geometry/Interfaces/IIntersectable.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Typed_Mesh.hpp>

using namespace Mlib;

IntersectableResource::IntersectableResource(
    std::list<TypedMesh<std::shared_ptr<IIntersectable>>>&& intersectables)
    : intersectables_{ std::move(intersectables) }
{}

void IntersectableResource::preload(const RenderableResourceFilter& filter) const
{}

std::shared_ptr<AnimatedColoredVertexArrays> IntersectableResource::get_arrays(
    const ColoredVertexArrayFilter& filter) const
{
    return std::make_shared<AnimatedColoredVertexArrays>();
}

std::list<TypedMesh<std::shared_ptr<IIntersectable>>> IntersectableResource::get_intersectables() const {
    return intersectables_;
}
