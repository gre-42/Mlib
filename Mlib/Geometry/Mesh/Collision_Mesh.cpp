#include "Collision_Mesh.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Interfaces/IIntersectable.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Typed_Mesh.hpp>

using namespace Mlib;

template <class TData>
CollisionMesh<TData>::CollisionMesh(
    const ColoredVertexArray<TData>& mesh,
    NormalVectorErrorBehavior zero_normal_behavior)
    : name{ mesh.name }
{
    quads.reserve(mesh.quads.size());
    mesh.quads_sphere(quads, zero_normal_behavior);

    triangles.reserve(mesh.triangles.size());
    mesh.triangles_sphere(triangles, zero_normal_behavior);

    lines = mesh.lines_sphere();
}

template <class TData>
CollisionMesh<TData>::CollisionMesh(
    std::string name,
    TypedMesh<std::shared_ptr<IIntersectable<TData>>> intersectable)
    : name{ name }
    , intersectable{ std::move(intersectable) }
{}

template <class TData>
CollisionMesh<TData>::~CollisionMesh() = default;

namespace Mlib {

template class CollisionMesh<float>;
template class CollisionMesh<double>;

}
