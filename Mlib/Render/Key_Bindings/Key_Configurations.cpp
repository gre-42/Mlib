#include "Key_Configurations.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Json/Misc.hpp>
#include <Mlib/Render/Key_Bindings/Key_Configuration.hpp>

namespace KeyConfigurationArgs {
    BEGIN_ARGUMENT_LIST;
    DECLARE_ARGUMENT(unique);
    DECLARE_ARGUMENT(id);
    DECLARE_ARGUMENT(key);
    DECLARE_ARGUMENT(mouse_button);
    DECLARE_ARGUMENT(gamepad_button);
    DECLARE_ARGUMENT(joystick_digital_axes);
    DECLARE_ARGUMENT(tap_button);
    DECLARE_ARGUMENT(cursor_axis);
    DECLARE_ARGUMENT(cursor_sign_and_scale);
    DECLARE_ARGUMENT(scroll_wheel_axis);
    DECLARE_ARGUMENT(scroll_wheel_sign_and_scale);
    DECLARE_ARGUMENT(not_key);
    DECLARE_ARGUMENT(not_mouse_button);
    DECLARE_ARGUMENT(not_gamepad_button);
    DECLARE_ARGUMENT(not_joystick_digital_axes);
    DECLARE_ARGUMENT(not_tap_button);
    DECLARE_ARGUMENT(joystick_analog_axes);
    DECLARE_ARGUMENT(key2);
    DECLARE_ARGUMENT(gamepad_button2);
    DECLARE_ARGUMENT(joystick_digital_axes2);
    DECLARE_ARGUMENT(tap_button2);
}

namespace JoystickDigitalAxisArgs {
    BEGIN_ARGUMENT_LIST;
    DECLARE_ARGUMENT(axis);
    DECLARE_ARGUMENT(sign);
}

namespace BaseGamepadAnalogAxisBindingArgs {
    BEGIN_ARGUMENT_LIST;
    DECLARE_ARGUMENT(axis);
    DECLARE_ARGUMENT(sign_and_scale);
    DECLARE_ARGUMENT(deadzone);
    DECLARE_ARGUMENT(exponent);
}

namespace Mlib {

void from_json(const nlohmann::json& j, JoystickDigitalAxis& obj)
{
    validate(j, JoystickDigitalAxisArgs::options);
    j.at(JoystickDigitalAxisArgs::axis).get_to(obj.joystick_axis);
    j.at(JoystickDigitalAxisArgs::sign).get_to(obj.joystick_axis_sign);
}

void from_json(const nlohmann::json& j, BaseGamepadAnalogAxisBinding& obj)
{
    JsonView jv{ j };
    jv.validate(BaseGamepadAnalogAxisBindingArgs::options);
    j.at(BaseGamepadAnalogAxisBindingArgs::axis).get_to(obj.axis);
    j.at(BaseGamepadAnalogAxisBindingArgs::sign_and_scale).get_to(obj.sign_and_scale);
    obj.deadzone = jv.at<float>(BaseGamepadAnalogAxisBindingArgs::deadzone, 0);
    obj.exponent = jv.at<float>(BaseGamepadAnalogAxisBindingArgs::exponent, 1);
    if (std::isnan(obj.deadzone) || (obj.deadzone < 0.f) || (obj.deadzone > 1.f)) {
        THROW_OR_ABORT("Joystick deadzone must be >= 0 and <= 1");
    }
}

}

using namespace Mlib;

KeyConfigurations::KeyConfigurations() = default;

KeyConfigurations::~KeyConfigurations() = default;

void KeyConfigurations::load(
    const std::string& filename,
    const std::string& fallback_filename)
{
    const std::string& fn = path_exists(filename)
        ? filename
        : fallback_filename;
    if (!path_exists(fn)) {
        THROW_OR_ABORT("Neither \"" + filename + "\" nor \"" + fallback_filename + "\" exist");
    }
    nlohmann::json j;
    {
        auto f = create_ifstream(fn);
        if (f->fail()) {
            THROW_OR_ABORT("Could not open file " + fn);
        }
        *f >> j;
        if (f->fail()) {
            THROW_OR_ABORT("Could not read from file " + fn);
        }
    }
    for (const auto& e : j) {
        validate(e, KeyConfigurationArgs::options);
        auto str = [&e](const std::string& key){
            return e.contains(key)
                ? e[key]
                : "";
        };
        auto digital_axes = [&e](const std::string& key){
            if (e.contains(key)) {
                return e[key].get<std::map<std::string, JoystickDigitalAxis>>();
            }
            return std::map<std::string, JoystickDigitalAxis>{};
        };
        auto analog_axes = [&e](const std::string& key){
            std::map<std::string, BaseGamepadAnalogAxisBinding> result;
            if (e.contains(key)) {
                return e[key].get<std::map<std::string, BaseGamepadAnalogAxisBinding>>();
            }
            return result;
        };
        std::string id = e.at(KeyConfigurationArgs::id);
        KeyConfiguration key_config{
            .base_combo = BaseKeyCombination{{{
                BaseKeyBinding{
                    .key = str(KeyConfigurationArgs::key),
                    .mouse_button = str(KeyConfigurationArgs::mouse_button),
                    .gamepad_button = str(KeyConfigurationArgs::gamepad_button),
                    .joystick_axes = digital_axes(KeyConfigurationArgs::joystick_digital_axes),
                    .tap_button = str(KeyConfigurationArgs::tap_button)}}},
                BaseKeyBinding{
                    .key = str(KeyConfigurationArgs::not_key),
                    .mouse_button = str(KeyConfigurationArgs::not_mouse_button),
                    .gamepad_button = str(KeyConfigurationArgs::not_gamepad_button),
                    .joystick_axes = digital_axes(KeyConfigurationArgs::not_joystick_digital_axes),
                    .tap_button = str(KeyConfigurationArgs::not_tap_button)}
            },
            .base_gamepad_analog_axes = {analog_axes(KeyConfigurationArgs::joystick_analog_axes)},
            .base_cursor_axis = {
                .axis = e.contains(KeyConfigurationArgs::cursor_axis) ? e[KeyConfigurationArgs::cursor_axis].get<size_t>() : SIZE_MAX,
                .sign_and_scale = e.contains(KeyConfigurationArgs::cursor_sign_and_scale) ? e[KeyConfigurationArgs::cursor_sign_and_scale].get<float>() : NAN,
            },
            .base_scroll_wheel_axis = {
                .axis = e.contains(KeyConfigurationArgs::scroll_wheel_axis) ? e[KeyConfigurationArgs::scroll_wheel_axis].get<size_t>() : SIZE_MAX,
                .sign_and_scale = e.contains(KeyConfigurationArgs::scroll_wheel_sign_and_scale) ? e[KeyConfigurationArgs::scroll_wheel_sign_and_scale].get<float>() : NAN,
            }
        };
        if (e.contains(KeyConfigurationArgs::key2) ||
            e.contains(KeyConfigurationArgs::gamepad_button2) ||
            e.contains(KeyConfigurationArgs::joystick_digital_axes2) ||
            e.contains(KeyConfigurationArgs::tap_button2))
        {
            key_config.base_combo.key_bindings.push_back(BaseKeyBinding{
                .key = str(KeyConfigurationArgs::key2),
                .gamepad_button = str(KeyConfigurationArgs::gamepad_button2),
                .joystick_axes = digital_axes(KeyConfigurationArgs::joystick_digital_axes2),
                .tap_button = str(KeyConfigurationArgs::tap_button2)});
        }
        if (!key_configurations_.insert({id, std::move(key_config)}).second) {
            THROW_OR_ABORT("Duplicate key config: \"" + id + '"');
        }
    }
}

void KeyConfigurations::insert(std::string id, const KeyConfiguration& key_configuration) {
    if (!key_configurations_.try_emplace(std::move(id), key_configuration).second) {
        THROW_OR_ABORT("Key configuration with ID \"" + id + "\" already exists");
    }
}

const KeyConfiguration& KeyConfigurations::get(const std::string& name) const {
    return key_configurations_.get(name);
}
