#pragma once
#include <Mlib/Array/Array_Forward.hpp>


namespace Mlib { namespace Cv {

Array<float> intrinsic_matrix_from_dimensions(
    float focal_length,
    const Array<float>& sensor_size,
    const ArrayShape& picture_shape);

}}
