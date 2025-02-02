#include "Scroll_Wheel_Movement.hpp"
#include <Mlib/Render/Key_Bindings/Key_Configuration.hpp>
#include <Mlib/Render/Key_Bindings/Key_Configurations.hpp>
#include <cmath>

using namespace Mlib;

ScrollWheelMovement::ScrollWheelMovement(
    CursorStates& cursor_states,
    const KeyConfigurations& key_configurations,
    std::string id)
    : incremental_movement_{ cursor_states }
    , key_configurations_{ key_configurations }
    , id_{ std::move(id) }
{}

ScrollWheelMovement::~ScrollWheelMovement() = default;

float ScrollWheelMovement::axis_alpha(float ncached) {
    if (id_.empty()) {
        return NAN;
    }
    const auto& key_combination = key_configurations_.get(id_);
    return incremental_movement_.axis_alpha(key_combination.base_scroll_wheel_axis, ncached);
}
