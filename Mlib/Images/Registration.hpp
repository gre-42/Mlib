#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib {

Array<float> patch_registration(
    const Array<float>& image0,
    const Array<float>& image1,
    const ArrayShape& max_window_shape,
    bool preserve_shape);

void flow_registration(
    const Array<float>& moving,
    const Array<float>& fixed,
    Array<float>& displacement,
    size_t window_size,
    size_t box_size,
    size_t max_displacement,
    size_t niterations);

Array<float> apply_displacement(
    const Array<float>& moving,
    const Array<float>& displacement);

}
