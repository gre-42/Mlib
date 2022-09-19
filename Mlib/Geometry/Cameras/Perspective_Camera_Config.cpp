#include "Perspective_Camera_Config.hpp"
#include <cmath>

using namespace Mlib;

float PerspectiveCameraConfig::dpi(int window_height) const {
    return window_height / (2 * std::atan(PerspectiveCameraConfig().y_fov / 2));
}
