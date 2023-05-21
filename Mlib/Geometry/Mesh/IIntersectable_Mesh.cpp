#include "IIntersectable_Mesh.hpp"
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>

using namespace Mlib;

bool IIntersectableMesh::intersects(const IIntersectableMesh& other) const {
    return intersects(other.bounding_sphere());
}
