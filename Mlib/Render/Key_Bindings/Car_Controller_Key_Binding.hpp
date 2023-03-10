#pragma once
#include <Mlib/Math/Interp.hpp>
#include <optional>
#include <string>

namespace Mlib {

class SceneNode;

struct CarControllerKeyBinding {
    std::string id;
    std::string role;
    SceneNode* node;
    std::optional<float> surface_power;
    std::optional<Interp<float>> tire_angle_interp;
    std::optional<float> ascend_velocity;
};

}
