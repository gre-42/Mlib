#include "Physics_Resource_Filter.hpp"
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>

using namespace Mlib;

bool PhysicsResourceFilter::matches(const ColoredVertexArray& cva) const {
    return
        any(cva.physics_material & PhysicsMaterial::ATTR_COLLIDE) &&
        resource_filter.matches(cva);
}
