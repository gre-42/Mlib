
#include "IIntersectable_Mesh.hpp"
#include <Mlib/Geometry/Primitives/Bounding_Sphere.hpp>

using namespace Mlib;

IIntersectableMesh::IIntersectableMesh() = default;

IIntersectableMesh::~IIntersectableMesh() = default;

bool IIntersectableMesh::intersects(const IIntersectableMesh& other) const {
    return intersects(other.bounding_sphere());
}
