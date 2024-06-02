#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>

namespace Mlib {

struct ImposterParameters {
    ImposterParameters(
        const AxisAlignedBoundingBox<float, 2>& sensor_aabb,
        const AxisAlignedBoundingBox<float, 2>& scaled_sensor_aabb,
        float distance_cam_to_obj);
    AxisAlignedBoundingBox<float, 2> pos = uninitialized;
    AxisAlignedBoundingBox<float, 2> uv = uninitialized;
};

}
