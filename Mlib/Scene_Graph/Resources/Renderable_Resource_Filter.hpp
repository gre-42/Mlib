#pragma once
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <iosfwd>

namespace Mlib {

template <class TPos>
class ColoredVertexArray;

struct RenderableResourceFilter {
    ~RenderableResourceFilter();
    template <class TPos>
    bool matches(size_t num, const ColoredVertexArray<TPos> &cva) const;
    size_t min_num = 0;
    size_t max_num = SIZE_MAX;
    ColoredVertexArrayFilter cva_filter;
};

std::ostream& operator << (std::ostream& ostr, const RenderableResourceFilter& filter);

}
