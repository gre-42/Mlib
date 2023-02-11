#pragma once

namespace Mlib {

class SceneNode;

struct CarControllerIdleBinding {
    SceneNode* node;
    float surface_power;
    float steer_angle;
    float drive_relaxation;
    float steer_relaxation;
};

}
