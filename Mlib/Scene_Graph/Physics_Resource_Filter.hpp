#pragma once
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Regex_Select.hpp>

namespace Mlib {

template <class TPos>
class ColoredVertexArray;

struct PhysicsResourceFilter {
#ifdef _MSC_VER
    PhysicsResourceFilter();
    ~PhysicsResourceFilter();
#endif
    template <class TPos>
    bool matches(const ColoredVertexArray<TPos>& cva) const;
    ColoredVertexArrayFilter cva_filter;
};

}
