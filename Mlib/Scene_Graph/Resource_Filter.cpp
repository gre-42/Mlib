#include "Resource_Filter.hpp"
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>

using namespace Mlib;

bool ResourceFilter::matches(const ColoredVertexArray& cva) const {
    return
        Mlib::re::regex_search(cva.name, include) &&
        !Mlib::re::regex_search(cva.name, exclude);
}
