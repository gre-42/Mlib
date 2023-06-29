#pragma once
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>

namespace Mlib {

template <class TPos>
class ColoredVertexArray;

struct PhysicsResourceFilter {
    ~PhysicsResourceFilter();
    template <class TPos>
    bool matches(const ColoredVertexArray<TPos>& cva) const;
    ColoredVertexArrayFilter cva_filter;
};

}
