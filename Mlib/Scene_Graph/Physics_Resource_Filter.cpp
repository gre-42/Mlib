#include "Physics_Resource_Filter.hpp"
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>

using namespace Mlib;

bool PhysicsResourceFilter::matches(const ColoredVertexArray& cva) const {
    return
        (cva.physics_material & PhysicsMaterial::COLLIDE) &&
        Mlib::re::regex_search(cva.name, include) &&
        !Mlib::re::regex_search(cva.name, exclude);
}
