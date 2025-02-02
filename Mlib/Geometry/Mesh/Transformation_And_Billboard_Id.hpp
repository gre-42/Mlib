#pragma once
#include <Mlib/Billboard_Id.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <cstdint>

namespace Mlib {

struct TransformationAndBillboardId {
    TransformationMatrix<float, float, 3> transformation_matrix;
    BillboardId billboard_id;
};

}
