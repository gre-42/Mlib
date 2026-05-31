#pragma once
#include <Mlib/OpenGL/Key_Bindings/Lockable_Key_Configurations_Fwd.hpp>
#include <Mlib/Scene_Config/Remote_Integers.hpp>
#include <cstdint>
#include <string>

namespace Mlib {

struct BaseAnalogAxisBinding;
class ButtonStates;

class GamepadAnalogAxesPosition {
    GamepadAnalogAxesPosition& operator = (const GamepadAnalogAxesPosition&) = delete;
public:
    GamepadAnalogAxesPosition(
        const ButtonStates& button_states,
        const LockableKeyConfigurations& key_configurations,
        NUserCountType user_id,
        std::string id,
        std::string role);
    ~GamepadAnalogAxesPosition();

    float axis_alpha();
private:
    const ButtonStates& button_states_;
    const LockableKeyConfigurations& key_configurations_;
    NUserCountType user_id_;
    std::string id_;
    std::string role_;
};

}
