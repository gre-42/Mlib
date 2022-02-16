#pragma once

namespace Mlib {

class SceneNode;

struct CarControllerIdleBinding {
    SceneNode* node;
    float surface_power;
    float steer_angle;
};

}
