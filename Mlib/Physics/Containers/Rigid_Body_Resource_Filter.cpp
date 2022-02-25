#include "Rigid_Body_Resource_Filter.hpp"
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>

using namespace Mlib;

bool RigidBodyResourceFilter::matches(const ColoredVertexArray& cva) const {
    return
        cva.material.collide &&
        Mlib::re::regex_search(cva.name, include) &&
        !Mlib::re::regex_search(cva.name, exclude);
}
