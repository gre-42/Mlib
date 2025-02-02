#pragma once

namespace Mlib {

struct OrthoCameraConfig {
    float near_plane = 1;
    float far_plane = 10000;
    float left_plane = -100;
    float right_plane = 100;
    float bottom_plane = -100;
    float top_plane = 100;
};

}
