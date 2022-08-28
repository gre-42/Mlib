#pragma once
#include <compare>
#include <iosfwd>
#include <string>

namespace Mlib {

struct BaseKeyBinding {
public:
    std::string key;
    std::string mouse_button;
    std::string gamepad_button;
    std::string joystick_axis;
    float joystick_axis_sign = 0.f;
    std::partial_ordering operator <=> (const BaseKeyBinding&) const = default;
};

std::ostream& operator << (std::ostream& ostr, const BaseKeyBinding& base_key_binding);

}
