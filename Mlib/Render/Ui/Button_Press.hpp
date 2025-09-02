#pragma once
#include <Mlib/Render/Key_Bindings/Lockable_Key_Configurations_Fwd.hpp>
#include <chrono>
#include <cstdint>
#include <string>

namespace Mlib {

struct BaseKeyBinding;
class ButtonStates;
struct ButtonStatesPrintArgs;

class ButtonPress {
    ButtonPress& operator = (const ButtonPress&) = delete;
public:
    ButtonPress(
        const ButtonStates& button_states,
        const LockableKeyConfigurations& key_configurations,
        uint32_t user_id,
        std::string id,
        std::string role);
    ~ButtonPress();

    void print(const ButtonStatesPrintArgs& args) const;

    bool keys_down() const;
    bool keys_pressed();
    float keys_alpha(float max_duration = 1.f);

private:
    const ButtonStates& button_states_;
    std::chrono::steady_clock::time_point keys_down_time_;
    bool key_was_up_;
    bool keys_down_;

    const LockableKeyConfigurations& key_configurations_;
    uint32_t user_id_;
    std::string id_;
    std::string role_;
};

}
