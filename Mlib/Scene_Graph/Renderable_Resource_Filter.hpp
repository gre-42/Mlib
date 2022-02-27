#pragma once
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Regex_Select.hpp>

namespace Mlib {

struct ColoredVertexArray;

struct RenderableResourceFilter {
    size_t min_num = 0;
    size_t max_num = SIZE_MAX;
    ColoredVertexArrayFilter cva_filter;
    bool matches(size_t num, const ColoredVertexArray& cva) const;
};

}
