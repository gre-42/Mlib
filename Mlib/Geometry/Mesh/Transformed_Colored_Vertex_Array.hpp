#pragma once
#include <Mlib/Geometry/Mesh/Transformation_And_Billboard_Id.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>

namespace Mlib {

struct TransformedColoredVertexArray {
    std::shared_ptr<ColoredVertexArray> cva;
    TransformationAndBillboardId trafo;
};

}
