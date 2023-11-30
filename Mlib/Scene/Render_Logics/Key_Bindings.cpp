#include "Key_Bindings.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Coordinates/To_Tait_Bryan_Angles.hpp>
#include <Mlib/Json/Misc.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Physics/Actuators/Engine_Power_Delta_Intent.hpp>
#include <Mlib/Physics/Actuators/Engine_Power_Intent.hpp>
#include <Mlib/Physics/Advance_Times/Gun.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Pitch_Look_At_Node.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Relative_Transformer.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Physics/Gravity.hpp>
#include <Mlib/Physics/Misc/Weapon_Cycle.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Avatar_Controllers/Rigid_Body_Avatar_Controller.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Car_Controllers/Rigid_Body_Vehicle_Controller.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Plane_Controllers/Rigid_Body_Plane_Controller.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Render/Key_Bindings/Absolute_Movable_Idle_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Absolute_Movable_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Avatar_Controller_Idle_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Avatar_Controller_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Base_Cursor_Axis_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Base_Gamepad_Analog_Axis_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Camera_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Car_Controller_Idle_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Car_Controller_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Gun_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Plane_Controller_Idle_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Plane_Controller_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Player_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Print_Node_Info_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Relative_Movable_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Weapon_Cycle_Key_Binding.hpp>
#include <Mlib/Render/Selected_Cameras/Camera_Cycle_Type.hpp>
#include <Mlib/Render/Selected_Cameras/Selected_Cameras.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Render/Ui/Cursor_Movement.hpp>
#include <Mlib/Render/Ui/Cursor_States.hpp>
#include <Mlib/Render/Ui/Gamepad_Analog_Axes_Position.hpp>
#include <Mlib/Scene_Graph/Animation/Animation_State_Updater.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>

using json = nlohmann::json;

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
DECLARE_ARGUMENT(not_tap_button2);
DECLARE_ARGUMENT(joystick_analog_axes);
DECLARE_ARGUMENT(key2);
DECLARE_ARGUMENT(gamepad_button2);
DECLARE_ARGUMENT(joystick_digital_axes2);
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
}

namespace Mlib {

struct KeyConfiguration {
    BaseKeyCombination base_combo;
    BaseGamepadAnalogAxesBinding base_gamepad_analog_axes;
    BaseCursorAxisBinding base_cursor_axis;
    BaseCursorAxisBinding base_scroll_wheel_axis;
    std::shared_ptr<CursorMovement> cursor_movement;
    std::shared_ptr<CursorMovement> scroll_wheel_movement;
};

class IncrementalAlpha {
public:
    explicit IncrementalAlpha(const KeyConfiguration& key_config)
        : key_config_{ key_config }
        , cursor_consumed_{ false }
        , scroll_wheel_consumed_{ false }
    {}
    ~IncrementalAlpha() {
        if (!cursor_consumed_ && (key_config_.cursor_movement != nullptr)) {
            key_config_.cursor_movement->axis_alpha(key_config_.base_cursor_axis);
        }
        if (!scroll_wheel_consumed_ && (key_config_.scroll_wheel_movement != nullptr)) {
            key_config_.scroll_wheel_movement->axis_alpha(key_config_.base_scroll_wheel_axis);
        }
    }
    float cursor_alpha() {
        cursor_consumed_ = true;
        if (key_config_.cursor_movement == nullptr) {
            return NAN;
        }
        return key_config_.cursor_movement->axis_alpha(key_config_.base_cursor_axis);
    }
    float scroll_wheel_alpha() {
        scroll_wheel_consumed_ = true;
        if (key_config_.scroll_wheel_movement == nullptr) {
            return NAN;
        }
        return key_config_.scroll_wheel_movement->axis_alpha(key_config_.base_scroll_wheel_axis);
    }
private:
    const KeyConfiguration& key_config_;
    bool cursor_consumed_;
    bool scroll_wheel_consumed_;
};

void from_json(const json& j, JoystickDigitalAxis& obj)
{
    validate(j, JoystickDigitalAxisArgs::options);
    j.at(JoystickDigitalAxisArgs::axis).get_to(obj.joystick_axis);
    j.at(JoystickDigitalAxisArgs::sign).get_to(obj.joystick_axis_sign);
}

void from_json(const json& j, BaseGamepadAnalogAxisBinding& obj)
{
    validate(j, BaseGamepadAnalogAxisBindingArgs::options);
    j.at(BaseGamepadAnalogAxisBindingArgs::axis).get_to(obj.axis);
    j.at(BaseGamepadAnalogAxisBindingArgs::sign_and_scale).get_to(obj.sign_and_scale);
}

}

using namespace Mlib;

KeyBindings::KeyBindings(
    ButtonPress& button_press,
    bool print_gamepad_buttons,
    GamepadAnalogAxesPosition& gamepad_analog_axes_position,
    CursorStates& cursor_states,
    CursorStates& scroll_wheel_states,
    SelectedCameras& selected_cameras,
    const Focuses& focuses,
    Players& players)
: button_press_{button_press},
  print_gamepad_buttons_{print_gamepad_buttons},
  gamepad_analog_axes_position_{gamepad_analog_axes_position},
  cursor_states_{cursor_states},
  scroll_wheel_states_{scroll_wheel_states},
  selected_cameras_{selected_cameras},
  focuses_{focuses},
  players_{players}
{}

KeyBindings::~KeyBindings() {
    std::set<DanglingPtr<SceneNode>> nodes;
    for (auto& b : absolute_movable_idle_bindings_) { nodes.insert(b.node); }
    for (auto& b : absolute_movable_key_bindings_) { nodes.insert(b.node); }
    for (auto& b : relative_movable_key_bindings_) { if (b.fixed_node != nullptr) nodes.insert(b.fixed_node); }
    for (auto& b : car_controller_idle_bindings_) { nodes.insert(b.node); }
    for (auto& b : car_controller_key_bindings_) { nodes.insert(b.node); }
    for (auto& b : plane_controller_idle_bindings_) { nodes.insert(b.node); }
    for (auto& b : plane_controller_key_bindings_) { nodes.insert(b.node); }
    for (auto& b : avatar_controller_idle_bindings_) { nodes.insert(b.node); }
    for (auto& b : avatar_controller_key_bindings_) { nodes.insert(b.node); }
    for (auto& b : weapon_cycle_key_bindings_) { nodes.insert(b.node); }
    for (auto& b : gun_key_bindings_) { nodes.insert(b.node); }
    for (auto& b : player_key_bindings_) { nodes.insert(b.node); }
    for (auto& node : nodes) {
        node->clearing_observers.remove(*this);
    }
}

void KeyBindings::notify_destroyed(DanglingRef<const SceneNode> destroyed_object) {
    auto dop = destroyed_object.ptr();
    absolute_movable_idle_bindings_.remove_if([&dop](const auto& b){return b.node == dop;});
    absolute_movable_key_bindings_.remove_if([&dop](const auto& b){return b.node == dop;});
    relative_movable_key_bindings_.remove_if([&dop](const auto& b){return b.fixed_node == dop;});
    car_controller_idle_bindings_.remove_if([&dop](const auto& b){return b.node == dop;});
    car_controller_key_bindings_.remove_if([&dop](const auto& b){return b.node == dop;});
    plane_controller_idle_bindings_.remove_if([&dop](const auto& b){return b.node == dop;});
    plane_controller_key_bindings_.remove_if([&dop](const auto& b){return b.node == dop;});
    avatar_controller_idle_bindings_.remove_if([&dop](const auto& b){return b.node == dop;});
    avatar_controller_key_bindings_.remove_if([&dop](const auto& b){return b.node == dop;});
    weapon_cycle_key_bindings_.remove_if([&dop](const auto& b){return b.node == dop;});
    gun_key_bindings_.remove_if([&dop](const auto& b){return b.node == dop;});
    player_key_bindings_.remove_if([&dop](const auto& b){return b.node == dop;});
}

void KeyBindings::load_key_configurations(
        const std::string& filename,
        const std::string& fallback_filename)
{
    const std::string& fn = path_exists(filename)
        ? filename
        : fallback_filename;
    if (!path_exists(fn)) {
        THROW_OR_ABORT("Neither \"" + filename + "\" nor \"" + fallback_filename + "\" exist");
    }
    json j;
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
            .base_combo = BaseKeyCombination{
                {{
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
                    .tap_button = str(KeyConfigurationArgs::not_tap_button)}},
            .base_gamepad_analog_axes = {analog_axes(KeyConfigurationArgs::joystick_analog_axes)},
            .base_cursor_axis = {
                .axis = e.contains(KeyConfigurationArgs::cursor_axis) ? e[KeyConfigurationArgs::cursor_axis].get<size_t>() : SIZE_MAX,
                .sign_and_scale = e.contains(KeyConfigurationArgs::cursor_sign_and_scale) ? e[KeyConfigurationArgs::cursor_sign_and_scale].get<float>() : NAN,
            },
            .base_scroll_wheel_axis = {
                .axis = e.contains(KeyConfigurationArgs::scroll_wheel_axis) ? e[KeyConfigurationArgs::scroll_wheel_axis].get<size_t>() : SIZE_MAX,
                .sign_and_scale = e.contains(KeyConfigurationArgs::scroll_wheel_sign_and_scale) ? e[KeyConfigurationArgs::scroll_wheel_sign_and_scale].get<float>() : NAN,
            },
            .cursor_movement = e.contains(KeyConfigurationArgs::cursor_axis)
                ? std::make_shared<CursorMovement>(cursor_states_)
                : nullptr,
            .scroll_wheel_movement = e.contains(KeyConfigurationArgs::scroll_wheel_axis)
                ? std::make_shared<CursorMovement>(scroll_wheel_states_)
                : nullptr
        };
        if (e.contains(KeyConfigurationArgs::key2) ||
            e.contains(KeyConfigurationArgs::gamepad_button2) ||
            e.contains(KeyConfigurationArgs::joystick_digital_axes2))
        {
            key_config.base_combo.key_bindings.push_back(BaseKeyBinding{
                .key = str(KeyConfigurationArgs::key2),
                .gamepad_button = str(KeyConfigurationArgs::gamepad_button2),
                .joystick_axes = digital_axes(KeyConfigurationArgs::joystick_digital_axes2),
                .tap_button = str(KeyConfigurationArgs::not_tap_button2)});
        }
        if (!key_configurations_.insert({id, std::move(key_config)}).second) {
            THROW_OR_ABORT("Duplicate key config: \"" + id + '"');
        }
    }
}

void KeyBindings::add_camera_key_binding(const CameraKeyBinding& b) {
    camera_key_bindings_.push_back(b);
}

const AbsoluteMovableIdleBinding& KeyBindings::add_absolute_movable_idle_binding(const AbsoluteMovableIdleBinding& b) {
    b.node->clearing_observers.add(*this, ObserverAlreadyExistsBehavior::IGNORE);
    return absolute_movable_idle_bindings_.emplace_back(b);
}

const AbsoluteMovableKeyBinding& KeyBindings::add_absolute_movable_key_binding(const AbsoluteMovableKeyBinding& b) {
    b.node->clearing_observers.add(*this, ObserverAlreadyExistsBehavior::IGNORE);
    return absolute_movable_key_bindings_.emplace_back(b);
}

const RelativeMovableKeyBinding& KeyBindings::add_relative_movable_key_binding(const RelativeMovableKeyBinding& b) {
    if (b.fixed_node != nullptr) {
        b.fixed_node->clearing_observers.add(*this, ObserverAlreadyExistsBehavior::IGNORE);
    }
    return relative_movable_key_bindings_.emplace_back(b);
}

const CarControllerIdleBinding& KeyBindings::add_car_controller_idle_binding(const CarControllerIdleBinding& b) {
    b.node->clearing_observers.add(*this, ObserverAlreadyExistsBehavior::IGNORE);
    return car_controller_idle_bindings_.emplace_back(b);
}

const CarControllerKeyBinding& KeyBindings::add_car_controller_key_binding(const CarControllerKeyBinding& b) {
    b.node->clearing_observers.add(*this, ObserverAlreadyExistsBehavior::IGNORE);
    return car_controller_key_bindings_.emplace_back(b);
}

const PlaneControllerIdleBinding& KeyBindings::add_plane_controller_idle_binding(const PlaneControllerIdleBinding& b) {
    b.node->clearing_observers.add(*this, ObserverAlreadyExistsBehavior::IGNORE);
    return plane_controller_idle_bindings_.emplace_back(b);
}

const PlaneControllerKeyBinding& KeyBindings::add_plane_controller_key_binding(const PlaneControllerKeyBinding& b) {
    b.node->clearing_observers.add(*this, ObserverAlreadyExistsBehavior::IGNORE);
    return plane_controller_key_bindings_.emplace_back(b);
}

const AvatarControllerIdleBinding& KeyBindings::add_avatar_controller_idle_binding(const AvatarControllerIdleBinding& b) {
    b.node->clearing_observers.add(*this, ObserverAlreadyExistsBehavior::IGNORE);
    return avatar_controller_idle_bindings_.emplace_back(b);
}

const AvatarControllerKeyBinding& KeyBindings::add_avatar_controller_key_binding(const AvatarControllerKeyBinding& b) {
    b.node->clearing_observers.add(*this, ObserverAlreadyExistsBehavior::IGNORE);
    return avatar_controller_key_bindings_.emplace_back(b);
}

const WeaponCycleKeyBinding& KeyBindings::add_weapon_inventory_key_binding(const WeaponCycleKeyBinding& b) {
    b.node->clearing_observers.add(*this, ObserverAlreadyExistsBehavior::IGNORE);
    return weapon_cycle_key_bindings_.emplace_back(b);
}

const GunKeyBinding& KeyBindings::add_gun_key_binding(const GunKeyBinding& b) {
    b.node->destruction_observers.add(*this, ObserverAlreadyExistsBehavior::IGNORE);
    return gun_key_bindings_.emplace_back(b);
}

const PlayerKeyBinding& KeyBindings::add_player_key_binding(const PlayerKeyBinding& b) {
    b.node->clearing_observers.add(*this, ObserverAlreadyExistsBehavior::IGNORE);
    return player_key_bindings_.emplace_back(b);
}

const PrintNodeInfoKeyBinding& KeyBindings::add_print_node_info_key_binding(const PrintNodeInfoKeyBinding& b) {
    if (b.fixed_node != nullptr) {
        b.fixed_node->clearing_observers.add(*this, ObserverAlreadyExistsBehavior::IGNORE);
    }
    return print_node_info_key_bindings_.emplace_back(b);
}

void KeyBindings::delete_relative_movable_key_binding(const RelativeMovableKeyBinding& deleted_key_binding) {
    relative_movable_key_bindings_.remove_if([&deleted_key_binding](const auto& b){return &b == &deleted_key_binding;});
}

void KeyBindings::delete_absolute_movable_idle_binding(const AbsoluteMovableIdleBinding& deleted_key_binding) {
    absolute_movable_idle_bindings_.remove_if([&deleted_key_binding](const auto& b){return &b == &deleted_key_binding;});
}

void KeyBindings::delete_absolute_movable_key_binding(const AbsoluteMovableKeyBinding& deleted_key_binding) {
    absolute_movable_key_bindings_.remove_if([&deleted_key_binding](const auto& b){return &b == &deleted_key_binding;});
}

void KeyBindings::delete_car_controller_idle_binding(const CarControllerIdleBinding& deleted_key_binding) {
    car_controller_idle_bindings_.remove_if([&deleted_key_binding](const auto& b){return &b == &deleted_key_binding;});
}

void KeyBindings::delete_car_controller_key_binding(const CarControllerKeyBinding& deleted_key_binding) {
    car_controller_key_bindings_.remove_if([&deleted_key_binding](const auto& b){return &b == &deleted_key_binding;});
}

void KeyBindings::delete_plane_controller_idle_binding(const PlaneControllerIdleBinding& deleted_key_binding) {
    plane_controller_idle_bindings_.remove_if([&deleted_key_binding](const auto& b){return &b == &deleted_key_binding;});
}

void KeyBindings::delete_plane_controller_key_binding(const PlaneControllerKeyBinding& deleted_key_binding) {
    plane_controller_key_bindings_.remove_if([&deleted_key_binding](const auto& b){return &b == &deleted_key_binding;});
}

void KeyBindings::delete_avatar_controller_idle_binding(const AvatarControllerIdleBinding& deleted_key_binding) {
    avatar_controller_idle_bindings_.remove_if([&deleted_key_binding](const auto& b){return &b == &deleted_key_binding;});
}

void KeyBindings::delete_avatar_controller_key_binding(const AvatarControllerKeyBinding& deleted_key_binding) {
    avatar_controller_key_bindings_.remove_if([&deleted_key_binding](const auto& b){return &b == &deleted_key_binding;});
}

void KeyBindings::delete_weapon_cycle_key_binding(const WeaponCycleKeyBinding& deleted_key_binding) {
    weapon_cycle_key_bindings_.remove_if([&deleted_key_binding](const auto& b){return &b == &deleted_key_binding;});
}

void KeyBindings::delete_gun_key_binding(const GunKeyBinding& deleted_key_binding) {
    gun_key_bindings_.remove_if([&deleted_key_binding](const auto& b){return &b == &deleted_key_binding;});
}

void KeyBindings::delete_player_key_binding(const PlayerKeyBinding& deleted_key_binding) {
    player_key_bindings_.remove_if([&deleted_key_binding, this](const auto& b){
        if (&b == &deleted_key_binding) {
            key_configurations_.get(b.id).base_combo.destruction_observers.reset();
            return true;
        }
        return false;
    });
}

void KeyBindings::delete_print_node_info_key_binding(const PrintNodeInfoKeyBinding& deleted_key_binding) {
    print_node_info_key_bindings_.remove_if([&deleted_key_binding](const auto& b){return &b == &deleted_key_binding;});
}

float KeyBindings::get_alpha(const KeyConfiguration& key_config, const std::string& role, IncrementalAlpha& incremental_alpha)
{
    // Analog gamepad axis
    float alpha = gamepad_analog_axes_position_.axis_alpha(key_config.base_gamepad_analog_axes, role);
    if (std::isnan(alpha) || std::abs(alpha) < 0.1f) {
        // Digital button
        {
            float alpha_digital = button_press_.keys_alpha(key_config.base_combo, role, 0.05f);
            if (!std::isnan(alpha_digital)) {
                return alpha_digital;
            }
        }
        // Analog cursor
        {
            float alpha_digital = incremental_alpha.cursor_alpha();
            if (!std::isnan(alpha_digital)) {
                return alpha_digital;
            }
        }
        // Analog scroll wheel
        {
            float alpha_digital = incremental_alpha.scroll_wheel_alpha();
            if (!std::isnan(alpha_digital)) {
                return alpha_digital;
            }
        }
    }
    return alpha;
}

void KeyBindings::increment_external_forces(
    const std::list<RigidBodyVehicle*>& olist,
    bool burn_in,
    const PhysicsEngineConfig& cfg)
{
    if (burn_in) {
        return;
    }
    {
        std::shared_lock lock{focuses_.mutex};
        if (focuses_.focus() != Focus::SCENE) {
            return;
        }
    }
    // if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    //     glfwSetWindowShouldClose(window_, GLFW_TRUE);
    // }
    if (print_gamepad_buttons_) {
        button_press_.print();
    }
    Map<std::string, IncrementalAlpha> incremental_alphas;
    for (const auto& [k, v] : key_configurations_) {
        if (!incremental_alphas.try_emplace(k, v).second) {
            verbose_abort("KeyBindings::increment_external_forces: Internal error");
        }
    }
    // std::cerr << std::endl;
    // std::cerr << std::endl;
    // for (size_t i = 0; i < 15; ++i) {
    //     std::cerr << i << "=" << (uint)gamepad_state.buttons[i] << " ";
    // }
    // std::cerr << std::endl;
    // for (size_t i = 0; i < 6; ++i) {
    //     std::cerr << i << "=" << gamepad_state.axes[i] << " ";
    // }
    // std::cerr << std::endl;

    // Absolute movable
    for (const auto& k : absolute_movable_idle_bindings_) {
        auto rb = dynamic_cast<RigidBodyVehicle*>(&k.node->get_absolute_movable());
        if (rb == nullptr) {
            THROW_OR_ABORT("Absolute movable is not a rigid body");
        }
        rb->set_surface_power("main", EnginePowerIntent{.surface_power = 0});
        rb->set_surface_power("brakes", EnginePowerIntent{.surface_power = 0});
        rb->set_max_velocity(INFINITY);
        for (auto& t : rb->tires_) {
            t.second.angle_y = 0;
            // t.second.accel_x = 0;
        }
        rb->tires_z_ = k.tires_z;
    }
    for (const auto& k : absolute_movable_key_bindings_) {
        float alpha = button_press_.keys_alpha(key_configurations_.get(k.id).base_combo, k.role, 0.05f);
        if (!std::isnan(alpha)) {
            auto rb = dynamic_cast<RigidBodyVehicle*>(&k.node->get_absolute_movable());
            if (rb == nullptr) {
                THROW_OR_ABORT("Absolute movable is not a rigid body");
            }
            if (any(k.force.vector != 0.f)) {
                rb->integrate_force(rb->abs_F(k.force), cfg);
            }
            if (any(k.rotate != 0.f)) {
                rb->rbi_.rbp_.rotation_ = dot2d(rb->rbi_.rbp_.rotation_, rodrigues1(alpha * k.rotate));
            }
            if (k.car_surface_power.has_value()) {
                rb->set_surface_power("main", EnginePowerIntent{.surface_power = k.car_surface_power.value()});
                rb->set_surface_power("brakes", EnginePowerIntent{.surface_power = k.car_surface_power.value()});
            }
            if (k.max_velocity != INFINITY) {
                rb->set_max_velocity(k.max_velocity);
            }
            if (k.tire_id != SIZE_MAX) {
                if (false) {
                    float a = gravity_magnitude * 1.f;
                    float l = 2.55f;
                    float r = sum(squared(rb->rbi_.rbp_.v_)) / a;
                    float angle = std::asin(std::clamp(l / r, 0.f, 1.f));
                    rb->set_tire_angle_y(k.tire_id, angle * alpha * sign(k.tire_angle_interp(0)));
                }
                // float v = std::sqrt(sum(squared(rb->rbi_.rbp_.v_)));
                float v = std::abs(dot0d(
                    rb->rbi_.rbp_.v_,
                    rb->rbi_.rbp_.rotation_.column(2)));
                rb->set_tire_angle_y(k.tire_id, alpha * degrees * k.tire_angle_interp(v * 3.6f));
                // rb->set_tire_accel_x(k.tire_id, alpha * sign(k.tire_angle_interp(0)));
            }
            if (any(k.tires_z != 0.f)) {
                rb->tires_z_ += k.tires_z;
            }
            if ((alpha == 0) && k.wants_to_jump.has_value() && k.wants_to_jump.value()) {
                rb->set_wants_to_jump();
            }
            if (k.wants_to_grind.has_value()) {
                rb->grind_state_.wants_to_grind_ = k.wants_to_grind.value();
            }
            if (k.fly_forward_factor.has_value()) {
                rb->fly_forward_state_.wants_to_fly_forward_factor_ = k.fly_forward_factor.value();
            }
        }
    }
    for (const auto& k : absolute_movable_idle_bindings_) {
        auto rb = dynamic_cast<RigidBodyVehicle*>(&k.node->get_absolute_movable());
        if (rb == nullptr) {
            THROW_OR_ABORT("Absolute movable is not a rigid body");
        }
        if (any(abs(rb->tires_z_) > float(1e-12))) {
            rb->tires_z_ /= std::sqrt(sum(squared(rb->tires_z_)));
        } else {
            rb->tires_z_ = { 0.f, 0.f, 1.f };
            rb->set_surface_power("main", EnginePowerIntent{.surface_power = NAN});
            rb->set_surface_power("brakes", EnginePowerIntent{.surface_power = NAN});
        }
        if (rb->animation_state_updater_ != nullptr) {
            rb->animation_state_updater_->notify_movement_intent();
        }
    }
    // Relative movable
    {
        std::vector<std::pair<const RelativeMovableKeyBinding&, DanglingRef<SceneNode>>> v;
        v.reserve(relative_movable_key_bindings_.size());
        for (auto& k : relative_movable_key_bindings_) {
            auto node = k.dynamic_node();
            if (node == nullptr) {
                continue;
            }
            v.emplace_back(k, *node);
        }
        for (const auto& [k, node] : v) {
            auto& m = node->get_relative_movable();
            auto rt = dynamic_cast<RelativeTransformer*>(&m);
            auto ypln = dynamic_cast<YawPitchLookAtNodes*>(&m);

            // Reset to defaults
            if (rt != nullptr) {
                rt->w_ = 0.f;
            } else if (ypln != nullptr) {
                // Do nothing (yet)
            } else {
                THROW_OR_ABORT("Relative movable is neither a relative transformer nor yaw/pitch-look-at-nodes");
            }
        }
        for (const auto& [k, node] : v) {
            auto& m = node->get_relative_movable();
            auto rt = dynamic_cast<RelativeTransformer*>(&m);
            auto ypln = dynamic_cast<YawPitchLookAtNodes*>(&m);

            auto rotate = [&k=k, &rt, &ypln](float dangle){
                if (dangle == 0.f) {
                    return;
                }
                if (rt != nullptr) {
                    // rt->w_ = w * k.rotation_axis;
                    // auto r = node->rotation();
                    auto z = rt->transformation_matrix_.R().column(2);
                    auto r = FixedArray<float, 3>{z_to_pitch(z), z_to_yaw(z), 0.f};
                    if (all(k.rotation_axis == FixedArray<float, 3>{0.f, 1.f, 0.f})) {
                        r(1) += dangle;
                    } else if (all(k.rotation_axis == FixedArray<float, 3>{1.f, 0.f, 0.f})) {
                        r(0) += dangle;
                    } else {
                        THROW_OR_ABORT("Unsupported rotation axis for relative transformer");
                    }
                    rt->transformation_matrix_.R() = tait_bryan_angles_2_matrix(r);
                } else if (ypln != nullptr) {
                    if (all(k.rotation_axis == FixedArray<float, 3>{0.f, 1.f, 0.f})) {
                        ypln->increment_yaw(dangle);
                    } else if (all(k.rotation_axis == FixedArray<float, 3>{1.f, 0.f, 0.f})) {
                        ypln->pitch_look_at_node().increment_pitch(dangle);
                    } else {
                        THROW_OR_ABORT("Unsupported rotation axis for yaw/pitch-look-at-nodes");
                    }
                } else {
                    THROW_OR_ABORT("Relative movable is neither a relative transformer nor yaw/pitch-look-at-nodes");
                }
            };

            auto translate = [&k=k, &rt](double dx){
                if (dx == 0.) {
                    return;
                }
                if (rt != nullptr) {
                    rt->transformation_matrix_.t() += dx * dot1d(rt->transformation_matrix_.R().casted<double>(), k.translation);
                }
            };

            // Apply key binding
            const auto& key_config = key_configurations_.get(k.id);
            float alpha = button_press_.keys_alpha(key_config.base_combo, k.role);
            if (!std::isnan(alpha)) {
                double v = ((1 - alpha) * k.velocity_press + alpha * k.velocity_repeat);
                float w = ((1 - alpha) * k.angular_velocity_press + alpha * k.angular_velocity_repeat);
                translate(v * cfg.dt);
                rotate(w * cfg.dt);
            }
            if (key_config.cursor_movement != nullptr) {
                float beta = key_config.cursor_movement->axis_alpha(key_config.base_cursor_axis);
                if (!std::isnan(beta)) {
                    rotate(beta * k.speed_cursor);
                    translate(beta * k.speed_cursor);
                }
            }
        }
    }
    // Node info
    for (const auto& k : print_node_info_key_bindings_) {
        auto node = k.dynamic_node();
        if (node == nullptr) {
            continue;
        }
        const auto& key_config = key_configurations_.get(k.id);
        if (button_press_.keys_pressed(key_config.base_combo, k.role)) {
            linfo() << "Key ID: " << k.id;
            auto trafo = node->absolute_model_matrix();
            auto z = trafo.R().column(2);
            linfo() << "Position: " << trafo.t() / (double)meters;
            linfo() << "Pitch: " << z_to_pitch(z) / degrees;
            linfo() << "Yaw: " << z_to_yaw(z) / degrees;
        }
    }
    // Avatar controller
    for (const auto& k : avatar_controller_idle_bindings_) {
        auto rb = dynamic_cast<RigidBodyVehicle*>(&k.node->get_absolute_movable());
        if (rb == nullptr) {
            THROW_OR_ABORT("Absolute movable is not a rigid body");
        }
        rb->avatar_controller().reset();
    }
    for (const auto& k : avatar_controller_key_bindings_) {
        auto rb = dynamic_cast<RigidBodyVehicle*>(&k.node->get_absolute_movable());
        if (rb == nullptr) {
            THROW_OR_ABORT("Absolute movable is not a rigid body");
        }
        const auto& key_config = key_configurations_.get(k.id);
        float alpha = button_press_.keys_alpha(key_config.base_combo, k.role, 0.05f);
        if (!std::isnan(alpha)) {
            if (k.surface_power.has_value()) {
                rb->avatar_controller().walk(k.surface_power.value());
                rb->avatar_controller().increment_legs_z(k.legs_z.value());
            }
            if (k.angular_velocity_press.has_value() && k.angular_velocity_repeat.has_value()) {
                float w = ((1 - alpha) * k.angular_velocity_press.value() + alpha * k.angular_velocity_repeat.value());
                if (k.yaw) {
                    rb->avatar_controller().increment_yaw(w * cfg.dt);
                }
                if (k.pitch) {
                    rb->avatar_controller().increment_pitch(w * cfg.dt);
                }
            }
        }
        if (key_config.cursor_movement != nullptr) {
            float beta = key_config.cursor_movement->axis_alpha(key_config.base_cursor_axis);
            if (!std::isnan(beta) && k.speed_cursor.has_value()) {
                float dangle = beta * k.speed_cursor.value();
                if (k.yaw) {
                    rb->avatar_controller().increment_yaw(dangle);
                }
                if (k.pitch) {
                    rb->avatar_controller().increment_pitch(dangle);
                }
            }
        }
    }
    for (const auto& k : avatar_controller_idle_bindings_) {
        auto rb = dynamic_cast<RigidBodyVehicle*>(&k.node->get_absolute_movable());
        if (rb == nullptr) {
            THROW_OR_ABORT("Absolute movable is not a rigid body");
        }
        rb->avatar_controller().apply();
    }
    // Vehicle controller
    for (const auto& k : car_controller_idle_bindings_) {
        auto rb = dynamic_cast<RigidBodyVehicle*>(&k.node->get_absolute_movable());
        if (rb == nullptr) {
            THROW_OR_ABORT("Absolute movable is not a rigid body");
        }
        rb->vehicle_controller().reset_parameters(
            k.surface_power,
            k.steer_angle);
        rb->vehicle_controller().reset_relaxation(
            k.drive_relaxation,
            k.steer_relaxation);
    }
    for (const auto& k : car_controller_key_bindings_) {
        float alpha = get_alpha(
            key_configurations_.get(k.id),
            k.role,
            incremental_alphas.get(k.id));
        if (!std::isnan(alpha)) {
            auto rb = dynamic_cast<RigidBodyVehicle*>(&k.node->get_absolute_movable());
            if (rb == nullptr) {
                THROW_OR_ABORT("Absolute movable is not a rigid body");
            }
            if (k.surface_power.has_value()) {
                rb->vehicle_controller().drive(
                    k.surface_power.value(),
                    alpha);
            }
            if (k.tire_angle_interp.has_value()) {
                float v = std::abs(dot0d(
                    rb->rbi_.rbp_.v_,
                    rb->rbi_.rbp_.rotation_.column(2)));
                rb->vehicle_controller().steer(k.tire_angle_interp.value()(v), alpha);
            }
            if (k.ascend_velocity.has_value()) {
                rb->vehicle_controller().ascend_by(k.ascend_velocity.value() * cfg.dt);
            }
        }
    }
    for (const auto& k : car_controller_idle_bindings_) {
        auto rb = dynamic_cast<RigidBodyVehicle*>(&k.node->get_absolute_movable());
        if (rb == nullptr) {
            THROW_OR_ABORT("Absolute movable is not a rigid body");
        }
        rb->vehicle_controller().apply();
    }
    // Plane controller
    for (const auto& k : plane_controller_idle_bindings_) {
        auto rb = dynamic_cast<RigidBodyVehicle*>(&k.node->get_absolute_movable());
        if (rb == nullptr) {
            THROW_OR_ABORT("Absolute movable is not a rigid body");
        }
        rb->plane_controller().reset_parameters(0.f, 0.f, 0.f, 0.f, 0.f);
        rb->plane_controller().reset_relaxation(0.f, 0.f, 0.f, 0.f);
    }
    for (const auto& k : plane_controller_key_bindings_) {
        float alpha = get_alpha(
            key_configurations_.get(k.id),
            k.role,
            incremental_alphas.get(k.id));
        if (!std::isnan(alpha)) {
            auto rb = dynamic_cast<RigidBodyVehicle*>(&k.node->get_absolute_movable());
            if (rb == nullptr) {
                THROW_OR_ABORT("Absolute movable is not a rigid body");
            }
            if (k.turbine_power.has_value()) {
                rb->plane_controller().accelerate(k.turbine_power.value(), alpha);
            }
            if (k.brake.has_value()) {
                rb->plane_controller().brake(k.brake.value(), alpha);
            }
            if (k.pitch.has_value()) {
                rb->plane_controller().pitch(alpha * k.pitch.value(), alpha);
            }
            if (k.yaw.has_value()) {
                rb->plane_controller().yaw(alpha * k.yaw.value(), alpha);
            }
            if (k.roll.has_value()) {
                rb->plane_controller().roll(alpha * k.roll.value(), alpha);
            }
        }
    }
    for (const auto& k : plane_controller_idle_bindings_) {
        auto rb = dynamic_cast<RigidBodyVehicle*>(&k.node->get_absolute_movable());
        if (rb == nullptr) {
            THROW_OR_ABORT("Absolute movable is not a rigid body");
        }
        rb->plane_controller().apply();
    }
    // Weapon inventory
    for (auto& k : weapon_cycle_key_bindings_) {
        float alpha = get_alpha(
            key_configurations_.get(k.id),
            k.role,
            incremental_alphas.get(k.id));
        if (!std::isnan(alpha)) {
            auto wc = dynamic_cast<WeaponCycle*>(&k.node->get_node_modifier());
            if (wc == nullptr) {
                THROW_OR_ABORT("Node modifier is not a weapon cycle");
            }
            if (k.direction == 1) {
                wc->equip_next_weapon();
            } else if (k.direction == -1) {
                wc->equip_previous_weapon();
            } else {
                THROW_OR_ABORT("Weapon cycle direction not -1 or 1");
            }
        }
    }
    // Gun
    for (const auto& k : gun_key_bindings_) {
        const auto& key_config = key_configurations_.get(k.id);
        if (button_press_.keys_down(key_config.base_combo, k.role)) {
            auto gun = dynamic_cast<Gun*>(&k.node->get_absolute_observer());
            if (gun == nullptr) {
                THROW_OR_ABORT("Absolute observer is not a gun");
            }
            gun->trigger(k.player, &players_.get_team(k.player->team_name()));
        }
    }
    // Player
    for (const auto& k : player_key_bindings_) {
        if (button_press_.keys_pressed(key_configurations_.get(k.id).base_combo, k.role)) {
            auto rb = dynamic_cast<RigidBodyVehicle*>(&k.node->get_absolute_movable());
            if (rb == nullptr) {
                THROW_OR_ABORT("Absolute movable is not a rigid body");
            }
            if (rb->driver_ == nullptr) {
                THROW_OR_ABORT("Rigid body has no driver");
            }
            Player* player = dynamic_cast<Player*>(rb->driver_);
            if (player == nullptr) {
                THROW_OR_ABORT("Driver is not a player");
            }
            if (k.select_next_opponent) {
                player->select_next_opponent();
            }
            if (k.select_next_vehicle) {
                player->select_next_vehicle();
            }
        }
    }
}

void KeyBindings::render(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("KeyBindings::render");
    // Near camera
    for (const auto& k : camera_key_bindings_) {
        if (k.tpe == CameraCycleType::FAR) {
            continue;
        }
        const auto& key_config = key_configurations_.get(k.id);
        if (button_press_.keys_pressed(key_config.base_combo, k.role)) {
            selected_cameras_.cycle_camera(k.tpe);
        }
    }
}

void KeyBindings::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "KeyBindings\n";
}
