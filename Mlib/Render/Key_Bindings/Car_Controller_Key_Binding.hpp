#pragma once
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Combination.hpp>
#include <optional>

namespace Mlib {

class SceneNode;

struct CarControllerKeyBinding {
    BaseKeyCombination base_combo;
    SceneNode* node;
    std::optional<float> surface_power;
    std::optional<Interp<float>> tire_angle_interp;
    std::optional<float> ascend_velocity;
};

}
