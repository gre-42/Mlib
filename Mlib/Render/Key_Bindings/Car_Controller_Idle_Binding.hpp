#pragma once
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>

namespace Mlib {

class SceneNode;

struct CarControllerIdleBinding {
    DanglingPtr<SceneNode> node;
    float surface_power;
    float steer_angle;
    float drive_relaxation;
    float steer_relaxation;
};

}
