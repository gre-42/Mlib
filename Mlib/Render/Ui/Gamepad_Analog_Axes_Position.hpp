#pragma once
#include <string>

namespace Mlib {

struct BaseGamepadAnalogAxesBinding;
class ButtonStates;

class GamepadAnalogAxesPosition {
    GamepadAnalogAxesPosition& operator = (const GamepadAnalogAxesPosition&) = delete;
public:
    explicit GamepadAnalogAxesPosition(const ButtonStates& button_states);
    ~GamepadAnalogAxesPosition();

    float axis_alpha(
        const BaseGamepadAnalogAxesBinding& binding,
        const std::string& role = "");
private:
    const ButtonStates& button_states_;
};

}
