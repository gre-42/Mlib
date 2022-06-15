#pragma once
#include <Mlib/Math/Transformation_Matrix.hpp>

namespace Mlib {

struct TransformationAndBillboardId {
    TransformationMatrix<float, float, 3> transformation_matrix;
    uint32_t billboard_id;
};

}
