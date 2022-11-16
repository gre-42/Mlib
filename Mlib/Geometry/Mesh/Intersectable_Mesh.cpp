#include "Intersectable_Mesh.hpp"
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>

using namespace Mlib;

bool IntersectableMesh::intersects(const IntersectableMesh& other) const {
    return intersects(other.bounding_sphere());
}
