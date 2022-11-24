#pragma once

namespace Mlib {

struct BaseGamepadAnalogAxisBinding;
class ButtonStates;

class GamepadAnalogAxesPosition {
    GamepadAnalogAxesPosition& operator = (const GamepadAnalogAxesPosition&) = delete;
public:
    explicit GamepadAnalogAxesPosition(const ButtonStates& button_states);
    ~GamepadAnalogAxesPosition();

    float axis_alpha(const BaseGamepadAnalogAxisBinding& binding);
private:
    const ButtonStates& button_states_;
};

}
