#pragma once

namespace Mlib {

struct CameraConfig {
    float y_fov = 0.5;
    float aspect_ratio = 0.5;
    float near_plane = 1;
    float far_plane = 1000;
    float left_plane = -100;
    float right_plane = 100;
    float bottom_plane = -100;
    float top_plane = 100;
};

}
