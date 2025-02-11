#pragma once
#include <Mlib/Render/Key_Bindings/Base_Cursor_Axis_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Base_Gamepad_Analog_Axis_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Combination.hpp>
#include <Mlib/Render/Key_Bindings/Key_Configuration.hpp>
#include <string>

namespace Mlib {

struct KeyConfiguration {
    BaseKeyCombination base_combo;
    BaseAnalogAxesListBinding base_gamepad_analog_axes;
    BaseCursorAxisBinding base_cursor_axis;
    BaseCursorAxisBinding base_scroll_wheel_axis;
    std::string to_string() const;
};

}
