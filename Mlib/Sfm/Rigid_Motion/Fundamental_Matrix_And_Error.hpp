#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib::Sfm {

struct FundamentalMatrixAndError {
    FixedArray<float, 3, 3> F;
    float error;
};

}
