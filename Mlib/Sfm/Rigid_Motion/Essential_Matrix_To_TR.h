#pragma once

#include <Mlib/Math/Math.hpp>

namespace Mlib { namespace Sfm {

class EssentialMatrixToTR {
public:
    EssentialMatrixToTR(const Array<float>& E);
    Array<float> t0;
    Array<float> R0;
    Array<float> t1;
    Array<float> R1;
};

}}
