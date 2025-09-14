#include "Button_States.hpp"
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Input_Map/Gamepad_Button_Map.hpp>
#include <Mlib/Render/Input_Map/Joystick_Axes_Map.hpp>
#include <Mlib/Render/Input_Map/Key_Events.hpp>
#include <Mlib/Render/Input_Map/Key_Map.hpp>
#include <Mlib/Render/Input_Map/Mouse_Button_Map.hpp>
#include <Mlib/Render/Input_Map/Tap_Analog_Axes_Map.hpp>
#include <Mlib/Render/Input_Map/Tap_Button_Map.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Filter_Type.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <cmath>
#include <iostream>
#include <mutex>

using namespace Mlib;

// InputFilter::InputFilter()
//     : pid_{0.f, 2.f, 0.f, 0.01f}
// {}
// 
// void InputFilter::operator () (const float& e) {
//     xhat_ = std::clamp(pid_(e), -1.f, 1.f);
// }
// 
// std::optional<float> InputFilter::xhat() const {
//     return xhat_;
// }

static bool make_digital(float v, float sign_and_threshold) {
    if (std::isnan(v)) {
        return false;
    }
    if (sign(v) != sign(sign_and_threshold)) {
        return false;
    }
    return (std::abs(v) >= std::abs(sign_and_threshold));
}

#ifndef __ANDROID__
ButtonStates::ButtonStates()
    : tap_buttons_(16)
    , gamepad_state_{ {} }
    , has_gamepad_{ false }
{}
#else
ButtonStates::ButtonStates() = default;
#endif

ButtonStates::~ButtonStates() = default;

#ifndef __ANDROID__
float ButtonStates::get_gamepad_axis(
    uint32_t gamepad_id,
    int axis,
    FilterType filter_type) const
{
    std::shared_lock lock{ gamepad_state_mutex_ };
    static_assert(GLFW_JOYSTICK_1 == 0);
    if (gamepad_id > GLFW_JOYSTICK_LAST) {
        THROW_OR_ABORT("Unknown gamepad ID");
    }
    if (!has_gamepad_[gamepad_id]) {
        return NAN;
    }
    if ((size_t)axis >= (sizeof(gamepad_state_[0].axes) / sizeof(gamepad_state_[0].axes[0]))) {
        THROW_OR_ABORT("Unknown gamepad axis");
    }
    switch (filter_type) {
    case FilterType::NONE:
        return gamepad_state_[gamepad_id].axes[axis];
    case FilterType::FILTERED:
        auto res = gamepad_axes_[gamepad_id].at(axis).xhat();
        if (!res.has_value()) {
            verbose_abort("Internal error: No interpolated gamepad state available");
        }
        return *res;
    }
    THROW_OR_ABORT("Unknown filter type: " + std::to_string((int)filter_type));
}

bool ButtonStates::get_gamepad_button_down(uint32_t gamepad_id, int button) const {
    std::shared_lock lock{ gamepad_state_mutex_ };
    static_assert(GLFW_JOYSTICK_1 == 0);
    if (gamepad_id > GLFW_JOYSTICK_LAST) {
        THROW_OR_ABORT("Unknown gamepad ID");
    }
    if (!has_gamepad_[gamepad_id]) {
        return false;
    }
    if ((size_t)button >= (sizeof(gamepad_state_[0].buttons) / sizeof(gamepad_state_[0].buttons[0]))) {
        THROW_OR_ABORT("Unknown gamepad button");
    }
    return gamepad_state_[gamepad_id].buttons[button] == GLFW_PRESS;
}

void ButtonStates::update_gamepad_state() {
    std::scoped_lock lock{ gamepad_state_mutex_ };
    static_assert(GLFW_JOYSTICK_1 == 0);
    for (int i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; ++i) {
        GLFW_CHK(has_gamepad_[i] = glfwGetGamepadState(i, gamepad_state_ + i));
        if (has_gamepad_[i]) {
            for (auto [j, s] : tenumerate<uint32_t>(gamepad_state_[i].axes)) {
                gamepad_axes_[i][j](s);
            }
        }
    }
}
#else

bool ButtonStates::get_gamepad_button_down(uint32_t gamepad_id, int button) const {
    std::shared_lock lock{ gamepad_button_mutex_ };
    auto git = gamepad_buttons_.find(gamepad_id);
    if (git == gamepad_buttons_.end()) {
        return false;
    }
    auto it = git->second.find(button);
    if (it == git->second.end()) {
        return false;
    }
    return it->second;
}

float ButtonStates::get_gamepad_axis(uint32_t gamepad_id, int axis) const {
    std::shared_lock lock{ gamepad_axes_mutex_ };
    auto git = gamepad_axes_.find(gamepad_id);
    if (git == gamepad_axes_.end()) {
        return NAN;
    }
    auto it = git->second.find(axis);
    if (it == git->second.end()) {
        return NAN;
    }
    return it->second;
}

void ButtonStates::notify_gamepad_button(uint32_t gamepad_id, int axis, bool value) {
    std::scoped_lock lock{ gamepad_button_mutex_ };
    gamepad_buttons_[gamepad_id][axis] = value;
}

void ButtonStates::notify_gamepad_axis(uint32_t gamepad_id, int axis, float value) {
    std::scoped_lock lock{ gamepad_axes_mutex_ };
    gamepad_axes_[gamepad_id][axis] = value;
}
#endif

bool ButtonStates::get_gamepad_digital_axis(
    uint32_t gamepad_id,
    int axis,
    float sign_and_threshold) const
{
    return make_digital(get_gamepad_axis(gamepad_id, axis, FilterType::NONE), sign_and_threshold);
}

bool ButtonStates::get_tap_analog_digital_axis(
    uint32_t gamepad_id,
    int axis,
    float sign_and_threshold) const
{
    return make_digital(get_tap_joystick_axis(gamepad_id, axis), sign_and_threshold);
}

void ButtonStates::notify_key_event(int key, int action) {
    if (action == KEY_PRESS) {
        std::scoped_lock lock{ keys_mutex_ };
        keys_down_.insert(key);
    }
    if (action == KEY_RELEASE) {
        std::scoped_lock lock{ keys_mutex_ };
        keys_down_.erase(key);
    }
}

bool ButtonStates::get_key_down(int key) const {
    std::shared_lock lock{ keys_mutex_ };
    return keys_down_.contains(key);
}

void ButtonStates::notify_mouse_button_event(int button, int action) {
    if (action == KEY_PRESS) {
        std::scoped_lock lock{ mouse_button_mutex_ };
        mouse_buttons_down_.insert(button);
    }
    if (action == KEY_RELEASE) {
        std::scoped_lock lock{ mouse_button_mutex_ };
        mouse_buttons_down_.erase(button);
    }
}

bool ButtonStates::get_mouse_button_down(int button) const {
    std::shared_lock lock{ mouse_button_mutex_ };
    return mouse_buttons_down_.contains(button);
}

bool ButtonStates::get_tap_button_down(
    uint32_t gamepad_id,
    int button) const
{
    std::shared_lock lock{ tap_buttons_mutex_ };
    auto git = tap_buttons_.find(gamepad_id);
    if (git == tap_buttons_.end()) {
        return false;
    }
    auto it = git->second.button_down.find(button);
    if (it == git->second.button_down.end()) {
        // The tap button might not yet exist (it is created dynamically),
        // so this is not an error.
        return false;
    }
    return it->second;
}

float ButtonStates::get_tap_joystick_axis(
    uint32_t gamepad_id,
    int axis) const
{
    std::shared_lock lock{ tap_buttons_mutex_ };
    auto bit = tap_buttons_.find(gamepad_id);
    if (bit == tap_buttons_.end()) {
        return NAN;
    }
    auto it = bit->second.joystick_axis_position.find(axis);
    if (it == bit->second.joystick_axis_position.end()) {
        // The tap button might not yet exist (it is created dynamically),
        // so this is not an error.
        return NAN;
    }
    return it->second;
}

void ButtonStates::print(const ButtonStatesPrintArgs& args) const {
    print(linfo().ref(), args);
}

void ButtonStates::print(std::ostream& ostr, const ButtonStatesPrintArgs& args) const {
    for (const auto& [name, code] : keys_map) {
        if (get_key_down(code)) {
            ostr << name << " ";
        }
    }
#ifndef __ANDROID__
    ostr << "\n\n";
    std::shared_lock lock{ gamepad_state_mutex_ };
    static_assert(GLFW_JOYSTICK_1 == 0);
    for (size_t j = 0; j <= GLFW_JOYSTICK_LAST; ++j) {
        if (has_gamepad_[j]) {
            ostr << '\n';
            ostr << '\n';
            if (args.physical) {
                for (size_t i = 0; i < 15; ++i) {
                    if (args.only_pressed && !gamepad_state_[j].buttons[i]) {
                        continue;
                    }
                    ostr << i << "=" << (unsigned int)gamepad_state_[j].buttons[i] << " ";
                }
                ostr << '\n';
                for (size_t i = 0; i < 6; ++i) {
                    if (std::fabs(gamepad_state_[j].axes[i]) < args.min_deflection) {
                        continue;
                    }
                    ostr << i << "=" << gamepad_state_[j].axes[i] << " ";
                }
            } else {
                for (const auto& [n, b] : gamepad_buttons_map) {
                    if (!b.has_value()) {
                        continue;
                    }
                    if (args.only_pressed && !gamepad_state_[j].buttons[*b]) {
                        continue;
                    }
                    ostr << n << "=" << (unsigned int)gamepad_state_[j].buttons[*b] << " ";
                }
                ostr << '\n';
                for (const auto& [n, b] : joystick_axes_map) {
                    if (!b.has_value()) {
                        continue;
                    }
                    if (std::fabs(gamepad_state_[j].axes[*b]) < args.min_deflection) {
                        continue;
                    }
                    ostr << n << "=" << gamepad_state_[j].axes[*b] << " ";
                }
            }
            ostr << '\n';
        } else {
            ostr << "No gamepad attached: " << j << '\n';
        }
    }
#endif
}

bool ButtonStates::key_down(const BaseKeyBinding& k, const std::string& role) const {
    if (auto joystick_axis = k.get_joystick_axis(role); joystick_axis != nullptr) {
        if (const auto& j = joystick_axis->joystick; j.has_value()) {
            if (const auto& axis = joystick_axes_map.get(j->axis);
                    axis.has_value() &&
                    get_gamepad_digital_axis(j->gamepad_id, *axis, j->sign_and_threshold))
            {
                return true;
            }
        }
        if (const auto& j = joystick_axis->tap; j.has_value()) {
            auto axis = tap_analog_axes_map.get(j->axis);
            if (get_tap_analog_digital_axis(j->gamepad_id, axis, j->sign_and_threshold)) {
                return true;
            }
        }
    }
    if (auto gamepad_button = k.get_gamepad_button(role); gamepad_button != nullptr) {
        auto button = gamepad_buttons_map.get(gamepad_button->button);
        if (button.has_value() && get_gamepad_button_down(gamepad_button->gamepad_id, *button)) {
            return true;
        }
    }
    if (auto tap_button = k.get_tap_button(role); tap_button != nullptr) {
        auto button = tap_buttons_map.get(tap_button->button);
        if (get_tap_button_down(tap_button->gamepad_id, button)) {
            return true;
        }
    }
    return
        (!k.key.empty() && get_key_down(keys_map.get(k.key))) ||
        (!k.mouse_button.empty() && get_mouse_button_down(mouse_buttons_map.get(k.mouse_button)));
}
