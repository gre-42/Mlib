#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <optional>

namespace Mlib {

struct CameraSensorAndNPixels {
    AxisAlignedBoundingBox<float, 2> scaled_sensor_aabb;
    int width;
    int height;
};

std::optional<CameraSensorAndNPixels> npixels_for_dpi(
    const AxisAlignedBoundingBox<float, 2>& sensor_aabb,
    float dpi,
    uint32_t min_texture_size,
    uint32_t max_texture_size);

}
