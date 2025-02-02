#pragma once
#include <Mlib/Geometry/Mesh/Transformation_And_Billboard_Id.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <memory>

namespace Mlib {

template <class TPos>
class ColoredVertexArray;

struct TransformedColoredVertexArray {
    std::shared_ptr<ColoredVertexArray<float>> scva;
    TransformationAndBillboardId trafo;
};

}
