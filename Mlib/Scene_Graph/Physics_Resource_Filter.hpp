#pragma once
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array_Filter.hpp>
#include <Mlib/Regex_Select.hpp>

namespace Mlib {

struct ColoredVertexArray;

struct PhysicsResourceFilter {
    ColoredVertexArrayFilter cva_filter;
    bool matches(const ColoredVertexArray& cva) const;
};

}
