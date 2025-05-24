#include "Button_Press.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Combination.hpp>
#include <Mlib/Render/Key_Bindings/Key_Configuration.hpp>
#include <Mlib/Render/Key_Bindings/Lockable_Key_Configurations.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <cmath>

static const auto default_time = std::chrono::steady_clock::time_point();

using namespace Mlib;

ButtonPress::ButtonPress(
    const ButtonStates& button_states,
    const LockableKeyConfigurations& key_configurations,
    uint32_t user_id,
    std::string id,
    std::string role)
    : button_states_{ button_states }
    , key_was_up_{ false }
    , keys_down_{ false }
    , key_configurations_{ key_configurations }
    , user_id_{ user_id }
    , id_{ std::move(id) }
    , role_{ std::move(role) }
{}

ButtonPress::~ButtonPress() = default;

void ButtonPress::print(bool physical, bool only_pressed) const {
    button_states_.print(physical, only_pressed);
}

bool ButtonPress::keys_down() const {
    if (id_.empty()) {
        return false;
    }
    auto lock = key_configurations_.lock_shared();
    const auto* key_combination = lock->try_get(user_id_, id_);
    if (key_combination == nullptr) {
        return false;
    }
    for (const auto& kk : key_combination->base_combo.key_bindings) {
        if (!button_states_.key_down(kk, role_)) {
            return false;
        }
    }
    if (button_states_.key_down(key_combination->base_combo.not_key_binding, role_)) {
        return false;
    }
    return true;
}

bool ButtonPress::keys_pressed() {
    bool is_down = keys_down();
    key_was_up_ |= !is_down;
    // Do not report a key press unless the key was up for some time.
    if (is_down && !key_was_up_) {
        return false;
    }
    bool& old_is_down = keys_down_;
    bool result = is_down && !old_is_down;
    old_is_down = is_down;
    return result;
}

float ButtonPress::keys_alpha(float max_duration) {
    if (keys_down()) {
        std::chrono::duration<float> duration;
        if (keys_down_time_ == default_time) {
            keys_down_time_ = std::chrono::steady_clock::now();
            duration = std::chrono::duration<float>::zero();
        } else {
            duration = std::chrono::steady_clock::now() - keys_down_time_;
        }
        return std::min(duration.count() / max_duration, 1.f);
    } else {
        keys_down_time_ = default_time;
        return NAN;
    }
}
