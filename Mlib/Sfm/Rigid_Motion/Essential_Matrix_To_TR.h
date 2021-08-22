#pragma once

#include <Mlib/Math/Transformation_Matrix.hpp>

namespace Mlib::Sfm {

class EssentialMatrixToTR {
public:
    explicit EssentialMatrixToTR(const FixedArray<float, 3, 3>& E);
    TransformationMatrix<float, 3> ke0;
    TransformationMatrix<float, 3> ke1;
};

}
