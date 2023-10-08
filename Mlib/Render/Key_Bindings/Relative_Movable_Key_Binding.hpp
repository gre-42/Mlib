#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <functional>
#include <string>

namespace Mlib {

class SceneNode;

struct RelativeMovableKeyBinding {
    std::string id;
    std::string role;
    DanglingPtr<SceneNode> fixed_node;
    std::function<DanglingPtr<SceneNode>()> dynamic_node;
    FixedArray<double, 3> translation;
    FixedArray<float, 3> rotation_axis;
    double velocity_press;
    double velocity_repeat;
    float angular_velocity_press;
    float angular_velocity_repeat;
    float speed_cursor;
};

}
