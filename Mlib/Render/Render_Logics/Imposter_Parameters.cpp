#include "Imposter_Parameters.hpp"

using namespace Mlib;

ImposterParameters::ImposterParameters(
    const AxisAlignedBoundingBox<float, 2>& sensor_aabb,
    const AxisAlignedBoundingBox<float, 2>& scaled_sensor_aabb,
    float distance_cam_to_obj)
{
    auto fmin = sensor_aabb.min * distance_cam_to_obj;
    auto fmax = sensor_aabb.max * distance_cam_to_obj;
    auto sfmin = scaled_sensor_aabb.min * distance_cam_to_obj;
    auto sfmax = scaled_sensor_aabb.max * distance_cam_to_obj;
    auto sfsize = sfmax - sfmin;
    pos = AxisAlignedBoundingBox<float, 2>::from_min_max(fmin, fmax);
    uv = AxisAlignedBoundingBox<float, 2>::from_min_max(
        (fmin - sfmin) / sfsize,
        (fmax - sfmin) / sfsize);
}
