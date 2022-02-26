#include "Renderable_Resource_Filter.hpp"
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>

using namespace Mlib;

bool RenderableResourceFilter::matches(size_t num, const ColoredVertexArray& cva) const {
    return
        any(cva.physics_material & PhysicsMaterial::ATTR_VISIBLE) &&
        (num >= min_num) &&
        (num <= max_num) &&
        resource_filter.matches(cva);
}
