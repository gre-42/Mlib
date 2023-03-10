#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <optional>

namespace Mlib {

class SceneNode;

struct AvatarControllerKeyBinding {
    std::string id;
    std::string role;
    SceneNode* node;
    std::optional<float> surface_power;
    bool yaw;
    bool pitch;
    std::optional<float> angular_velocity_press;
    std::optional<float> angular_velocity_repeat;
    std::optional<float> speed_cursor;
    std::optional<FixedArray<float, 3>> legs_z;
};

}
