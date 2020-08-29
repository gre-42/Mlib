#pragma once
#include <Mlib/Array/Array_Forward.hpp>

namespace Mlib {

void optical_flow(
    const Array<float>& image0,
    const Array<float>& image1,
    const Array<float>* image2,
    const ArrayShape& window_shape,
    float max_displacement,
    Array<float>& flow,
    Array<bool>& mask);

}
