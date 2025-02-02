#pragma once

namespace Mlib {

struct PerspectiveCameraConfig {
    float dpi(float window_height) const;
    float y_fov = 0.5;
    float aspect_ratio = 0.5;
    float near_plane = 1;
    float far_plane = 10000;
};

}
