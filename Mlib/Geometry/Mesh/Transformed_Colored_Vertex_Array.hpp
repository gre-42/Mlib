#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

struct TransformedColoredVertexArray {
    std::shared_ptr<ColoredVertexArray> cva;
    FixedArray<float, 4, 4> transformation_matrix;
};

}
