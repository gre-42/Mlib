#pragma once
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <chrono>
#include <map>
#include <string>

namespace Mlib {

struct BaseKeyBinding;
struct BaseKeyCombination;
class ButtonStates;

class ButtonPress: public DestructionObserver {
    ButtonPress& operator = (const ButtonPress&) = delete;
public:
    explicit ButtonPress(const ButtonStates& button_states);
    ~ButtonPress();

    virtual void notify_destroyed(const Object& destroyed_object) override;

    void print(bool physical = false, bool only_pressed = false) const;

    bool key_down(const BaseKeyBinding& k, const std::string& role = "") const;

    bool keys_down(const BaseKeyCombination& k, const std::string& role = "") const;
    bool keys_pressed(const BaseKeyCombination& k, const std::string& role = "");
    float keys_alpha(const BaseKeyCombination& k, const std::string& role = "", float max_duration = 1);
private:
    const ButtonStates& button_states_;
    std::map<const BaseKeyCombination*, std::chrono::time_point<std::chrono::steady_clock>> keys_down_times_;
    std::map<const BaseKeyCombination*, bool> keys_down_;
};

}
