#pragma once
#include <chrono>
#include <map>
#include <string>

namespace Mlib {

struct BaseKeyBinding;
struct BaseKeyCombination;
class ButtonStates;

class ButtonPress {
public:
    explicit ButtonPress(const ButtonStates& button_states);
    ~ButtonPress();
    ButtonPress& operator = (const ButtonPress&) = delete;

    void print(bool physical = false, bool only_pressed = false) const;

    bool key_down(const BaseKeyBinding& k) const;
    bool key_pressed(const BaseKeyBinding& k);
    float key_alpha(const BaseKeyBinding& k, float max_duration = 1);

    bool keys_down(const BaseKeyCombination& k) const;
    bool keys_pressed(const BaseKeyCombination& k);
    float keys_alpha(const BaseKeyCombination& k, float max_duration = 1);
private:
    const ButtonStates& button_states_;
    std::map<BaseKeyCombination, std::chrono::time_point<std::chrono::steady_clock>> keys_down_times_;
};

}
