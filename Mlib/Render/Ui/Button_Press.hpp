#pragma once
#include <chrono>
#include <string>

namespace Mlib {

struct BaseKeyBinding;
class KeyConfigurations;
class ButtonStates;

class ButtonPress {
    ButtonPress& operator = (const ButtonPress&) = delete;
public:
    ButtonPress(
        const ButtonStates& button_states,
        const KeyConfigurations& key_configurations,
        std::string id,
        std::string role);
    ~ButtonPress();

    void print(bool physical = false, bool only_pressed = false) const;

    bool keys_down() const;
    bool keys_pressed();
    float keys_alpha(float max_duration = 1.f);

private:
    const ButtonStates& button_states_;
    const KeyConfigurations& key_configurations_;
    std::chrono::time_point<std::chrono::steady_clock> keys_down_time_;
    bool key_was_up_;
    bool keys_down_;
    std::string id_;
    std::string role_;
};

}
