#pragma once
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

struct PerspectiveCameraConfig {
    FixedArray<float, 2> dpi(const FixedArray<float, 2>& texture_size) const;
    float y_fov = 0.5;
    float aspect_ratio = 0.5;
    float near_plane = 1;
    float far_plane = 10000;
};

}
