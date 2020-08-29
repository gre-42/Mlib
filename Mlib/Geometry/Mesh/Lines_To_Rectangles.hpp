#pragma once
#include <Mlib/Array/Array_Forward.hpp>

namespace Mlib {

bool lines_to_rectangles(
    FixedArray<float, 2>& p00,
    FixedArray<float, 2>& p01,
    FixedArray<float, 2>& p10,
    FixedArray<float, 2>& p11,
    const FixedArray<float, 2>& aL,
    const FixedArray<float, 2>& aR,
    const FixedArray<float, 2>& b,
    const FixedArray<float, 2>& c,
    const FixedArray<float, 2>& dL,
    const FixedArray<float, 2>& dR,
    float width_aLb,
    float width_aRb,
    float width_bc,
    float width_cdL,
    float width_cdR);

}
