#include "Perspective_Camera_Config.hpp"
#include <cmath>

using namespace Mlib;

float PerspectiveCameraConfig::dpi(float window_height) const {
    return window_height / (2.f * std::atan(PerspectiveCameraConfig().y_fov / 2.f));
}
