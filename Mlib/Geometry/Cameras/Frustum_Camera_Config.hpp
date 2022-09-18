#pragma once

namespace Mlib {

struct FrustumCameraConfig {
    float near_plane = 1;
    float far_plane = 10000;
    float left = -1;
    float right = 1;
    float bottom = -1;
    float top = 1;
};

}
