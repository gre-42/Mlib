#pragma once
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Render/Key_Bindings/Base_Gamepad_Analog_Axis_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Combination.hpp>
#include <optional>

namespace Mlib {

class SceneNode;

struct PlaneControllerKeyBinding {
    BaseKeyCombination base_combo;
    BaseGamepadAnalogAxisBinding base_gamepad_analog_axis;
    SceneNode* node;
    std::optional<float> turbine_power;
    std::optional<float> brake;
    std::optional<float> pitch;
    std::optional<float> yaw;
    std::optional<float> roll;
};

}
