#include "Colored_Vertex_Array_Filter.hpp"
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Scene_Precision.hpp>

using namespace Mlib;

ColoredVertexArrayFilter::~ColoredVertexArrayFilter() = default;

template <class TPos>
bool ColoredVertexArrayFilter::matches(const ColoredVertexArray<TPos>& cva) const {
    auto n = cva.name.name();
    return
        !any(~cva.morphology.physics_material & included_tags) &&
        !any(cva.morphology.physics_material & excluded_tags) &&
        Mlib::re::regex_search(n.begin(), n.end(), included_names) &&
        !Mlib::re::regex_search(n.begin(), n.end(), excluded_names);
}

template bool ColoredVertexArrayFilter::matches(const ColoredVertexArray<float>& cva) const;
template bool ColoredVertexArrayFilter::matches(const ColoredVertexArray<CompressedScenePos>& cva) const;
