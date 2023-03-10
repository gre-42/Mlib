#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <string>

namespace Mlib {

class SceneNode;

struct RelativeMovableKeyBinding {
    std::string id;
    std::string role;
    SceneNode* node;
    FixedArray<float, 3> rotation_axis;
    float angular_velocity_press;
    float angular_velocity_repeat;
    float speed_cursor;
};

}
