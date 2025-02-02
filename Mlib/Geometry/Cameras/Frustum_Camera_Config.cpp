#include "Frustum_Camera_Config.hpp"
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>

using namespace Mlib;

FrustumCameraConfig FrustumCameraConfig::from_sensor_aabb(
    const AxisAlignedBoundingBox<float, 2>& sensor_aabb,
    float near_plane,
    float far_plane)
{
    return FrustumCameraConfig{
        .near_plane = near_plane,
        .far_plane = far_plane,
        .left = sensor_aabb.min(0) * near_plane,
        .right = sensor_aabb.max(0) * near_plane,
        .bottom = sensor_aabb.min(1) * near_plane,
        .top = sensor_aabb.max(1) * near_plane};
}
