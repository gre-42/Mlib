#include "Perspective_Camera_Config.hpp"
#include <Mlib/Math/Fixed_Math.hpp>

using namespace Mlib;

FixedArray<float, 2> PerspectiveCameraConfig::dpi(const FixedArray<float, 2>& texture_size) const {
    return {
        texture_size(0) / (2.f * std::atan(y_fov / 2.f)) / aspect_ratio,
        texture_size(1) / (2.f * std::atan(y_fov / 2.f))};
}
