#pragma once
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>

namespace Mlib {

struct TransformedColoredVertexArray {
    std::shared_ptr<ColoredVertexArray> cva;
    OrderableFixedArray<float, 4, 4> transformation_matrix;
};

}
