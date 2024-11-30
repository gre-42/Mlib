#include "Npixels_For_Dpi.hpp"

using namespace Mlib;

std::optional<CameraSensorAndNPixels> Mlib::npixels_for_dpi(
    const AxisAlignedBoundingBox<float, 2>& sensor_aabb,
    float dpi,
    uint32_t min_texture_size,
    uint32_t max_texture_size)
{
    CameraSensorAndNPixels result{
        .scaled_sensor_aabb = uninitialized
    };
    auto sensor_size = sensor_aabb.max - sensor_aabb.min;
    auto sensor_center = sensor_aabb.center();
    if (any(sensor_size * dpi > float(max_texture_size)) ||
        any(sensor_size * dpi < float(min_texture_size)))
    {
        return std::nullopt;
    }
    result.width = (1 << (int)std::ceil(std::log2(sensor_size(0) * dpi)));
    result.height = (1 << (int)std::ceil(std::log2(sensor_size(1) * dpi)));
    auto npixels = FixedArray<float, 2>{(float)result.width, (float)result.height};
    result.scaled_sensor_aabb = AxisAlignedBoundingBox<float, 2>::from_min_max(
        sensor_center - npixels / dpi / 2.f,
        sensor_center + npixels / dpi / 2.f);
    return result;
}
