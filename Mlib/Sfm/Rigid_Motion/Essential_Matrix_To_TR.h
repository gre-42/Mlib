#pragma once

#include <Mlib/Math/Transformation_Matrix.hpp>

namespace Mlib { namespace Sfm {

class EssentialMatrixToTR {
public:
    EssentialMatrixToTR(const FixedArray<float, 3, 3>& E);
    TransformationMatrix<float, 3> tm0;
    TransformationMatrix<float, 3> tm1;
};

}}
