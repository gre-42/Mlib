#include "Collision_Mesh.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Interfaces/IIntersectable.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Typed_Mesh.hpp>
#include <Mlib/Scene_Precision.hpp>

using namespace Mlib;

template <class TData>
CollisionMesh::CollisionMesh(const ColoredVertexArray<TData>& mesh)
    : name{ mesh.name.full_name() }
{
    quads.reserve(mesh.quads.size());
    mesh.quads_sphere(quads);

    triangles.reserve(mesh.triangles.size());
    mesh.triangles_sphere(triangles);

    lines = mesh.lines_sphere();
}

CollisionMesh::CollisionMesh(
    std::string name,
    TypedMesh<std::shared_ptr<IIntersectable>> intersectable)
    : name{ std::move(name) }
    , intersectable{ std::move(intersectable) }
{}

CollisionMesh::~CollisionMesh() = default;

namespace Mlib {

template CollisionMesh::CollisionMesh(const ColoredVertexArray<float>&);
template CollisionMesh::CollisionMesh(const ColoredVertexArray<CompressedScenePos>&);

}
