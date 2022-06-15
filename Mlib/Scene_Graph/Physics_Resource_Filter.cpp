#include "Physics_Resource_Filter.hpp"
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>

using namespace Mlib;

template <class TPos>
bool PhysicsResourceFilter::matches(const ColoredVertexArray<TPos>& cva) const {
    return
        any(cva.physics_material & PhysicsMaterial::ATTR_COLLIDE) &&
        cva_filter.matches(cva);
}

template bool PhysicsResourceFilter::matches(const ColoredVertexArray<float>& cva) const;
template bool PhysicsResourceFilter::matches(const ColoredVertexArray<double>& cva) const;
