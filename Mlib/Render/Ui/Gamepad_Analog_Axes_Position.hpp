#pragma once
#include <string>

namespace Mlib {

struct BaseAnalogAxisBinding;
class KeyConfigurations;
class ButtonStates;

class GamepadAnalogAxesPosition {
    GamepadAnalogAxesPosition& operator = (const GamepadAnalogAxesPosition&) = delete;
public:
    GamepadAnalogAxesPosition(
        const ButtonStates& button_states,
        const KeyConfigurations& key_configurations,
        std::string id,
        std::string role);
    ~GamepadAnalogAxesPosition();

    float axis_alpha();
private:
    const ButtonStates& button_states_;
    const KeyConfigurations& key_configurations_;
    std::string id_;
    std::string role_;
};

}
