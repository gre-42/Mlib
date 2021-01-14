#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>

namespace Mlib {

struct TransformedColoredVertexArray {
    std::shared_ptr<ColoredVertexArray> cva;
    TransformationMatrix<float, 3> transformation_matrix;
};

}
