#include "Colored_Vertex_Array_Filter.hpp"
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>

using namespace Mlib;

template <class TPos>
bool ColoredVertexArrayFilter::matches(const ColoredVertexArray<TPos>& cva) const {
    return
        !any(~cva.physics_material & included_tags) &&
        !any(cva.physics_material & excluded_tags) &&
        Mlib::re::regex_search(cva.name, included_names) &&
        !Mlib::re::regex_search(cva.name, excluded_names);
}

template bool ColoredVertexArrayFilter::matches(const ColoredVertexArray<float>& cva) const;
template bool ColoredVertexArrayFilter::matches(const ColoredVertexArray<double>& cva) const;
