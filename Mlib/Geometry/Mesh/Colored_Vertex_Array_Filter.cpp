#include "Colored_Vertex_Array_Filter.hpp"
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>

using namespace Mlib;

bool ColoredVertexArrayFilter::matches(const ColoredVertexArray& cva) const {
    return
        !any(~cva.physics_material & included_tags) &&
        !any(cva.physics_material & excluded_tags) &&
        Mlib::re::regex_search(cva.name, included_names) &&
        !Mlib::re::regex_search(cva.name, excluded_names);
}
