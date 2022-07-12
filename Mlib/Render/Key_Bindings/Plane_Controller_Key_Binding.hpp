#pragma once
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Combination.hpp>
#include <optional>

namespace Mlib {

class SceneNode;

struct PlaneControllerKeyBinding {
    BaseKeyCombination base_combo;
    SceneNode* node;
    std::optional<float> turbine_power;
    std::optional<float> flaps_angle;
    std::optional<float> pitch;
    std::optional<float> yaw;
    std::optional<float> roll;
};

}
