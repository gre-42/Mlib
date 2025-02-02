#include "Renderable_Resource_Filter.hpp"
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>

using namespace Mlib;

RenderableResourceFilter::~RenderableResourceFilter()
{}

template <class TPos>
bool RenderableResourceFilter::matches(size_t num, const ColoredVertexArray<TPos>& cva) const {
    return
        any(cva.morphology.physics_material & PhysicsMaterial::ATTR_VISIBLE) &&
        (num >= min_num) &&
        (num <= max_num) &&
        cva_filter.matches(cva);
}

template bool RenderableResourceFilter::matches(size_t num, const ColoredVertexArray<float>& cva) const;
template bool RenderableResourceFilter::matches(size_t num, const ColoredVertexArray<CompressedScenePos>& cva) const;
