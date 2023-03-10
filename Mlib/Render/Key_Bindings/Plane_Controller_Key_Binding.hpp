#pragma once
#include <Mlib/Math/Interp.hpp>
#include <optional>

namespace Mlib {

class SceneNode;

struct PlaneControllerKeyBinding {
    std::string id;
    std::string role;
    SceneNode* node;
    std::optional<float> turbine_power;
    std::optional<float> brake;
    std::optional<float> pitch;
    std::optional<float> yaw;
    std::optional<float> roll;
};

}
