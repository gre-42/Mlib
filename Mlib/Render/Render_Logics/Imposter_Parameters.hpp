#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>

namespace Mlib {

struct FrustumCameraConfig;

struct ImposterParameters {
    ImposterParameters(
        const FrustumCameraConfig& frustum,
        const FrustumCameraConfig& scaled_frustum);
    AxisAlignedBoundingBox<float, 2> pos;
    AxisAlignedBoundingBox<float, 2> uv;
};

}
