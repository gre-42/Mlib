#pragma once
#include <cstddef>

namespace Mlib {

template <class TData, size_t tndim>
class AxisAlignedBoundingBox;

struct FrustumCameraConfig {
    static FrustumCameraConfig from_sensor_aabb(
        const AxisAlignedBoundingBox<float, 2>& sensor_aabb,
        float near_plane,
        float far_plane);
    float near_plane = 1;
    float far_plane = 10000;
    float left = -1;
    float right = 1;
    float bottom = -1;
    float top = 1;
};

}
