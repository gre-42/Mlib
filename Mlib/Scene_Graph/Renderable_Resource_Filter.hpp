#pragma once
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Regex_Select.hpp>

namespace Mlib {

template <class TPos>
struct ColoredVertexArray;

struct RenderableResourceFilter {
    size_t min_num = 0;
    size_t max_num = SIZE_MAX;
    ColoredVertexArrayFilter cva_filter;
    template <class TPos>
    bool matches(size_t num, const ColoredVertexArray<TPos>& cva) const;
};

}
