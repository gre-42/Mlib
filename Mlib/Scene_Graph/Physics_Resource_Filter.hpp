#pragma once
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Regex_Select.hpp>

namespace Mlib {

template <class TPos>
struct ColoredVertexArray;

struct PhysicsResourceFilter {
    ColoredVertexArrayFilter cva_filter;
    template <class TPos>
    bool matches(const ColoredVertexArray<TPos>& cva) const;
};

}
