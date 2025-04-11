#include "Key_Bindings.hpp"
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Geometry/Coordinates/To_Tait_Bryan_Angles.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Physics/Actuators/Engine_Power_Delta_Intent.hpp>
#include <Mlib/Physics/Actuators/Engine_Power_Intent.hpp>
#include <Mlib/Physics/Actuators/Tire.hpp>
#include <Mlib/Physics/Advance_Times/Gun.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Pitch_Look_At_Node.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Relative_Transformer.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Yaw_Pitch_Look_At_Nodes.hpp>
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
#include <Mlib/Render/Key_Bindings/Key_Configuration.hpp>
#include <Mlib/Render/Key_Bindings/Lockable_Key_Configurations.hpp>
#include <Mlib/Render/Key_Bindings/Plane_Controller_Idle_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Plane_Controller_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Player_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Print_Node_Info_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Relative_Movable_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Weapon_Cycle_Key_Binding.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Selected_Cameras/Camera_Cycle_Type.hpp>
#include <Mlib/Render/Selected_Cameras/Selected_Cameras.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Render/Ui/Cursor_Movement.hpp>
#include <Mlib/Render/Ui/Cursor_States.hpp>
#include <Mlib/Render/Ui/Gamepad_Analog_Axes_Position.hpp>
#include <Mlib/Render/Ui/Scroll_Wheel_Movement.hpp>
#include <Mlib/Scene_Graph/Animation/Animation_State_Updater.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>

using namespace Mlib;

KeyBindings::KeyBindings(
    SelectedCameras& selected_cameras,
    const Focuses& focuses,
    Players& players)
    : selected_cameras_{ selected_cameras }
    , focuses_{ focuses }
    , players_{ players }
{}

KeyBindings::~KeyBindings() {
    on_destroy.clear();
    if (!absolute_movable_idle_bindings_.empty()) { lwarn() << absolute_movable_idle_bindings_.size() << " absolute_movable_idle_bindings remaining"; }
    if (!absolute_movable_key_bindings_.empty()) { lwarn() << absolute_movable_key_bindings_.size() << " absolute_movable_key_bindings remaining"; }
    if (!relative_movable_key_bindings_.empty()) { lwarn() << relative_movable_key_bindings_.size() << " relative_movable_key_bindings remaining"; }
    if (!car_controller_idle_bindings_.empty()) { lwarn() << car_controller_idle_bindings_.size() << " car_controller_idle_bindings remaining"; }
    if (!car_controller_key_bindings_.empty()) { lwarn() << car_controller_key_bindings_.size() << " car_controller_key_bindings remaining"; }
    if (!plane_controller_idle_bindings_.empty()) { lwarn() << plane_controller_idle_bindings_.size() << " plane_controller_idle_bindings remaining"; }
    if (!plane_controller_key_bindings_.empty()) { lwarn() << plane_controller_key_bindings_.size() << " plane_controller_key_bindings remaining"; }
    if (!avatar_controller_idle_bindings_.empty()) { lwarn() << avatar_controller_idle_bindings_.size() << " avatar_controller_idle_bindings remaining"; }
    if (!avatar_controller_key_bindings_.empty()) { lwarn() << avatar_controller_key_bindings_.size() << " avatar_controller_key_bindings remaining"; }
    if (!weapon_cycle_key_bindings_.empty()) { lwarn() << weapon_cycle_key_bindings_.size() << " weapon_cycle_key_bindings remaining"; }
    if (!gun_key_bindings_.empty()) { lwarn() << gun_key_bindings_.size() << " gun_key_bindings remaining"; }
    if (!player_key_bindings_.empty()) { lwarn() << player_key_bindings_.size() << " player_key_bindings remaining"; }
    if (!print_node_info_key_bindings_.empty()) { lwarn() << print_node_info_key_bindings_.size() << " print_node_info_key_bindings remaining"; }
}

CameraKeyBinding& KeyBindings::add_camera_key_binding(std::unique_ptr<CameraKeyBinding>&& b) {
    return *camera_key_bindings_.emplace_back(std::move(b));
}

AbsoluteMovableIdleBinding& KeyBindings::add_absolute_movable_idle_binding(std::unique_ptr<AbsoluteMovableIdleBinding>&& b) {
    return *absolute_movable_idle_bindings_.emplace_back(std::move(b));
}

AbsoluteMovableKeyBinding& KeyBindings::add_absolute_movable_key_binding(std::unique_ptr<AbsoluteMovableKeyBinding>&& b) {
    return *absolute_movable_key_bindings_.emplace_back(std::move(b));
}

RelativeMovableKeyBinding& KeyBindings::add_relative_movable_key_binding(std::unique_ptr<RelativeMovableKeyBinding>&& b) {
    return *relative_movable_key_bindings_.emplace_back(std::move(b));
}

CarControllerIdleBinding& KeyBindings::add_car_controller_idle_binding(std::unique_ptr<CarControllerIdleBinding>&& b) {
    return *car_controller_idle_bindings_.emplace_back(std::move(b));
}

CarControllerKeyBinding& KeyBindings::add_car_controller_key_binding(std::unique_ptr<CarControllerKeyBinding>&& b) {
    return *car_controller_key_bindings_.emplace_back(std::move(b));
}

PlaneControllerIdleBinding& KeyBindings::add_plane_controller_idle_binding(std::unique_ptr<PlaneControllerIdleBinding>&& b) {
    return *plane_controller_idle_bindings_.emplace_back(std::move(b));
}

PlaneControllerKeyBinding& KeyBindings::add_plane_controller_key_binding(std::unique_ptr<PlaneControllerKeyBinding>&& b) {
    return *plane_controller_key_bindings_.emplace_back(std::move(b));
}

AvatarControllerIdleBinding& KeyBindings::add_avatar_controller_idle_binding(std::unique_ptr<AvatarControllerIdleBinding>&& b) {
    return *avatar_controller_idle_bindings_.emplace_back(std::move(b));
}

AvatarControllerKeyBinding& KeyBindings::add_avatar_controller_key_binding(std::unique_ptr<AvatarControllerKeyBinding>&& b) {
    return *avatar_controller_key_bindings_.emplace_back(std::move(b));
}

WeaponCycleKeyBinding& KeyBindings::add_weapon_inventory_key_binding(std::unique_ptr<WeaponCycleKeyBinding>&& b) {
    return *weapon_cycle_key_bindings_.emplace_back(std::move(b));
}

GunKeyBinding& KeyBindings::add_gun_key_binding(std::unique_ptr<GunKeyBinding>&& b) {
    return *gun_key_bindings_.emplace_back(std::move(b));
}

PlayerKeyBinding& KeyBindings::add_player_key_binding(std::unique_ptr<PlayerKeyBinding>&& b) {
    return *player_key_bindings_.emplace_back(std::move(b));
}

PrintNodeInfoKeyBinding& KeyBindings::add_print_node_info_key_binding(std::unique_ptr<PrintNodeInfoKeyBinding>&& b) {
    return *print_node_info_key_bindings_.emplace_back(std::move(b));
}

void KeyBindings::delete_relative_movable_key_binding(const RelativeMovableKeyBinding& deleted_key_binding) {
    if (relative_movable_key_bindings_.remove_if([&deleted_key_binding](const auto& b){return b.get() == &deleted_key_binding;}) != 1) {
        verbose_abort("Could not remove exactly one \"relative movable key binding\"");
    }
}

void KeyBindings::delete_absolute_movable_idle_binding(const AbsoluteMovableIdleBinding& deleted_key_binding) {
    if (absolute_movable_idle_bindings_.remove_if([&deleted_key_binding](const auto& b){return b.get() == &deleted_key_binding;}) != 1) {
        verbose_abort("Could not remove exactly one \"absolute movable idle binding\"");
    }
}

void KeyBindings::delete_absolute_movable_key_binding(const AbsoluteMovableKeyBinding& deleted_key_binding) {
    if (absolute_movable_key_bindings_.remove_if([&deleted_key_binding](const auto& b){return b.get() == &deleted_key_binding;}) != 1) {
        verbose_abort("Could not remove exactly one \"absolute movable key binding\"");
    }
}

void KeyBindings::delete_car_controller_idle_binding(const CarControllerIdleBinding& deleted_key_binding) {
    if (car_controller_idle_bindings_.remove_if([&deleted_key_binding](const auto& b){return b.get() == &deleted_key_binding;}) != 1) {
        verbose_abort("Could not remove exactly one \"car controller idle binding\"");
    }
}

void KeyBindings::delete_car_controller_key_binding(const CarControllerKeyBinding& deleted_key_binding) {
    if (car_controller_key_bindings_.remove_if([&deleted_key_binding](const auto& b){return b.get() == &deleted_key_binding;}) != 1) {
        verbose_abort("Could not remove exactly one \"car controller key binding\"");
    }
}

void KeyBindings::delete_plane_controller_idle_binding(const PlaneControllerIdleBinding& deleted_key_binding) {
    if (plane_controller_idle_bindings_.remove_if([&deleted_key_binding](const auto& b){return b.get() == &deleted_key_binding;}) != 1) {
        verbose_abort("Could not remove exactly one \"plane controller idle binding\"");
    }
}

void KeyBindings::delete_plane_controller_key_binding(const PlaneControllerKeyBinding& deleted_key_binding) {
    if (plane_controller_key_bindings_.remove_if([&deleted_key_binding](const auto& b){return b.get() == &deleted_key_binding;}) != 1) {
        verbose_abort("Could not remove exactly one \"plane controller key binding\"");
    }
}

void KeyBindings::delete_avatar_controller_idle_binding(const AvatarControllerIdleBinding& deleted_key_binding) {
    if (avatar_controller_idle_bindings_.remove_if([&deleted_key_binding](const auto& b){return b.get() == &deleted_key_binding;}) != 1) {
        verbose_abort("Could not remove exactly one \"avatar controller idle binding\"");
    }
}

void KeyBindings::delete_avatar_controller_key_binding(const AvatarControllerKeyBinding& deleted_key_binding) {
    if (avatar_controller_key_bindings_.remove_if([&deleted_key_binding](const auto& b){return b.get() == &deleted_key_binding;}) != 1) {
        verbose_abort("Could not remove exactly one \"abatar controller key binding\"");
    }
}

void KeyBindings::delete_weapon_cycle_key_binding(const WeaponCycleKeyBinding& deleted_key_binding) {
    if (weapon_cycle_key_bindings_.remove_if([&deleted_key_binding](const auto& b){return b.get() == &deleted_key_binding;}) != 1) {
        verbose_abort("Could not remove exactly one \"weapon cycle key binding\"");
    }
}

void KeyBindings::delete_gun_key_binding(const GunKeyBinding& deleted_key_binding) {
    if (gun_key_bindings_.remove_if([&deleted_key_binding](const auto& b){return b.get() == &deleted_key_binding;}) != 1) {
        verbose_abort("Could not remove exactly one \"gun key binding\"");
    }
}

void KeyBindings::delete_player_key_binding(const PlayerKeyBinding& deleted_key_binding) {
    if (player_key_bindings_.remove_if([&deleted_key_binding](const auto& b){return b.get() == &deleted_key_binding;}) != 1) {
        verbose_abort("Could not remove exactly one \"player key binding\"");
    }
}

void KeyBindings::delete_print_node_info_key_binding(const PrintNodeInfoKeyBinding& deleted_key_binding) {
    if (print_node_info_key_bindings_.remove_if([&deleted_key_binding](const auto& b){return b.get() == &deleted_key_binding;}) != 1) {
        verbose_abort("Could not remove exactly one \"print node info key binding\"");
    }
}

static float get_alpha(
    ButtonPress& button_press,
    CursorMovement* cursor_movement,
    ScrollWheelMovement* scroll_wheel_movement,
    GamepadAnalogAxesPosition* gamepad_analog_axes_position,
    float press_factor,
    float repeat_factor,
    const PhysicsEngineConfig& cfg)
{
    float alpha = button_press.keys_alpha(0.05f);
    if (!std::isnan(alpha)) {
        alpha = press_factor * (1 - alpha) + repeat_factor * alpha;
    }
    auto update_alpha = [&alpha](float a) {
        alpha = std::isnan(alpha) ? a : std::isnan(a) ? alpha : std::max(alpha, a);
        };
    if (gamepad_analog_axes_position != nullptr) {
        update_alpha(gamepad_analog_axes_position->axis_alpha());
    }
    if (cursor_movement != nullptr) {
        update_alpha(cursor_movement->axis_alpha(cfg.ncached()));
    }
    if (scroll_wheel_movement != nullptr) {
        update_alpha(scroll_wheel_movement->axis_alpha(cfg.ncached()));
    }
    return alpha;
}

static const auto main_name = VariableAndHash<std::string>{ "main" };
static const auto brakes_name = VariableAndHash<std::string>{ "brakes" };

void KeyBindings::increment_external_forces(
    const std::list<RigidBodyVehicle*>& olist,
    bool burn_in,
    const PhysicsEngineConfig& cfg,
    const StaticWorld& world)
{
    bool enable_controls = [&]() {
        if (burn_in) {
            return false;
        }
        {
            std::shared_lock lock{ focuses_.mutex };
            if (!focuses_.has_focus(Focus::SCENE)) {
                return false;
            }
        }
        return true;
    }();
    // if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    //     glfwSetWindowShouldClose(window_, GLFW_TRUE);
    // }
    // {
    //     err = lerr();
    //     err << std::endl;
    //     err << std::endl;
    //     for (size_t i = 0; i < 15; ++i) {
    //         err << i << "=" << (uint)gamepad_state.buttons[i] << " ";
    //     }
    //     err << std::endl;
    //     for (size_t i = 0; i < 6; ++i) {
    //         err << i << "=" << gamepad_state.axes[i] << " ";
    //     }
    //     err << std::endl;
    // }

    // Absolute movable
    if (enable_controls) {
        for (const auto& k : absolute_movable_idle_bindings_) {
            auto& rb = get_rigid_body_vehicle(*k->node);
            rb.set_surface_power(main_name, EnginePowerIntent{.surface_power = 0});
            rb.set_surface_power(brakes_name, EnginePowerIntent{.surface_power = 0});
            rb.set_max_velocity(INFINITY);
            for (auto& t : rb.tires_) {
                t.second.angle_y = 0;
                // t.second.accel_x = 0;
            }
            rb.tires_z_ = k->tires_z;
        }
    }
    for (auto& k : absolute_movable_key_bindings_) {
        float alpha = k->button_press.keys_alpha(0.05f);
        if (enable_controls && !std::isnan(alpha)) {
            auto& rb = get_rigid_body_vehicle(*k->node);
            if (any(k->force.vector != 0.f)) {
                rb.integrate_force(rb.abs_F(k->force), cfg);
            }
            if (any(k->rotate != 0.f)) {
                rb.rbp_.rotation_ = dot2d(rb.rbp_.rotation_, rodrigues1(alpha * k->rotate));
            }
            if (k->car_surface_power.has_value()) {
                rb.set_surface_power(main_name, EnginePowerIntent{.surface_power = *k->car_surface_power});
                rb.set_surface_power(brakes_name, EnginePowerIntent{.surface_power = *k->car_surface_power});
            }
            if (k->max_velocity != INFINITY) {
                rb.set_max_velocity(k->max_velocity);
            }
            if (k->tire_id != SIZE_MAX) {
                // float v = std::sqrt(sum(squared(rb->rbp_.v_)));
                float v = std::abs(dot0d(
                    rb.rbp_.v_,
                    rb.rbp_.rotation_.column(2)));
                rb.set_tire_angle_y(k->tire_id, alpha * degrees * k->tire_angle_interp(v));
                // rb->set_tire_accel_x(k->tire_id, alpha * sign(k->tire_angle_interp(0)));
            }
            if (any(k->tires_z != 0.f)) {
                rb.tires_z_ += k->tires_z;
            }
            if ((alpha == 0) && k->wants_to_jump.has_value() && *k->wants_to_jump) {
                rb.set_wants_to_jump();
            }
            if (k->wants_to_grind.has_value()) {
                rb.grind_state_.wants_to_grind_ = *k->wants_to_grind;
            }
            if (k->fly_forward_factor.has_value()) {
                rb.fly_forward_state_.wants_to_fly_forward_factor_ = *k->fly_forward_factor;
            }
        }
    }
    if (enable_controls) {
        for (const auto& k : absolute_movable_idle_bindings_) {
            auto& rb = get_rigid_body_vehicle(*k->node);
            if (any(abs(rb.tires_z_) > float(1e-12))) {
                rb.tires_z_ /= std::sqrt(sum(squared(rb.tires_z_)));
            } else {
                rb.tires_z_ = { 0.f, 0.f, 1.f };
                rb.set_surface_power(main_name, EnginePowerIntent{.surface_power = NAN});
                rb.set_surface_power(brakes_name, EnginePowerIntent{.surface_power = NAN});
            }
            if (rb.animation_state_updater_ != nullptr) {
                rb.animation_state_updater_->notify_movement_intent();
            }
        }
    }
    // Relative movable
    {
        std::vector<std::pair<RelativeMovableKeyBinding&, DanglingRef<SceneNode>>> k_n;
        k_n.reserve(relative_movable_key_bindings_.size());
        for (auto& k : relative_movable_key_bindings_) {
            auto node = k->dynamic_node();
            if (node == nullptr) {
                continue;
            }
            k_n.emplace_back(*k, *node);
        }
        if (enable_controls) {
            for (const auto& [k, node] : k_n) {
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
        }
        for (auto& [k, node] : k_n) {
            auto& m = node->get_relative_movable();
            auto rt = dynamic_cast<RelativeTransformer*>(&m);
            auto ypln = dynamic_cast<YawPitchLookAtNodes*>(&m);

            auto rotate = [&k=k, &rt, &ypln](float dangle){
                if (dangle == 0.f) {
                    return;
                }
                if (rt != nullptr) {
                    // rt->w_ = w * k->rotation_axis;
                    // auto r = node->rotation();
                    auto z = rt->transformation_matrix_.R.column(2);
                    auto r = FixedArray<float, 3>{z_to_pitch(z), z_to_yaw(z), 0.f};
                    if (all(k.rotation_axis == FixedArray<float, 3>{0.f, 1.f, 0.f})) {
                        r(1) += dangle;
                    } else if (all(k.rotation_axis == FixedArray<float, 3>{1.f, 0.f, 0.f})) {
                        r(0) += dangle;
                    } else {
                        THROW_OR_ABORT("Unsupported rotation axis for relative transformer");
                    }
                    rt->transformation_matrix_.R = tait_bryan_angles_2_matrix(r);
                } else if (ypln != nullptr) {
                    if (all(k.rotation_axis == FixedArray<float, 3>{0.f, 1.f, 0.f})) {
                        ypln->increment_yaw(dangle, 1.f);
                    } else if (all(k.rotation_axis == FixedArray<float, 3>{1.f, 0.f, 0.f})) {
                        ypln->pitch_look_at_node().increment_pitch(dangle, 1.f);
                    } else {
                        THROW_OR_ABORT("Unsupported rotation axis for yaw/pitch-look-at-nodes");
                    }
                } else {
                    THROW_OR_ABORT("Relative movable is neither a relative transformer nor yaw/pitch-look-at-nodes");
                }
            };

            auto translate = [&k=k, &rt](ScenePos dx){
                if (dx == 0.) {
                    return;
                }
                if (rt != nullptr) {
                    rt->transformation_matrix_.t += dx * dot1d(rt->transformation_matrix_.R.casted<ScenePos>(), k.translation);
                }
            };

            // Apply key binding
            float alpha = k.button_press.keys_alpha();
            if (enable_controls && !std::isnan(alpha)) {
                ScenePos v = ((1 - alpha) * k.velocity_press + alpha * k.velocity_repeat);
                float w = ((1 - alpha) * k.angular_velocity_press + alpha * k.angular_velocity_repeat);
                translate(v * cfg.dt_substeps());
                rotate(w * cfg.dt_substeps());
            }
            if (k.cursor_movement != nullptr) {
                float beta = k.cursor_movement->axis_alpha(cfg.ncached());
                if (enable_controls && !std::isnan(beta)) {
                    rotate(beta * k.speed_cursor);
                    translate(beta * k.speed_cursor);
                }
            }
        }
    }
    // Node info
    for (auto& k : print_node_info_key_bindings_) {
        auto node = k->dynamic_node();
        if (node == nullptr) {
            continue;
        }
        if (k->button_press.keys_pressed()) {
            auto trafo = node->absolute_model_matrix();
            auto z = trafo.R.column(2);
            if (k->geographic_mapping != nullptr) {
                linfo() << "Position (lat, lon, height): " << std::setprecision(18) << k->geographic_mapping->transform(trafo.t);
            }
            linfo() << "Position: " << std::setprecision(18) << trafo.t / (ScenePos)meters;
            linfo() << "Pitch: " << z_to_pitch(z) / degrees;
            linfo() << "Yaw: " << z_to_yaw(z) / degrees;
        }
    }
    // Avatar controller
    if (enable_controls) {
        for (const auto& k : avatar_controller_idle_bindings_) {
            auto& rb = get_rigid_body_vehicle(*k->node);
            rb.avatar_controller().reset();
        }
    }
    for (auto& k : avatar_controller_key_bindings_) {
        auto& rb = get_rigid_body_vehicle(*k->node);
        float alpha = get_alpha(
            k->button_press,
            k->cursor_movement.get(),
            nullptr,
            &k->gamepad_analog_axes_position,
            k->press_factor,
            k->repeat_factor,
            cfg);
        if (enable_controls && !std::isnan(alpha)) {
            if (k->surface_power.has_value()) {
                rb.avatar_controller().walk(*k->surface_power, alpha);
                rb.avatar_controller().increment_legs_z((*k->legs_z) * alpha);
            }
            if (k->angular_velocity.has_value()) {
                if (k->yaw) {
                    rb.avatar_controller().increment_yaw((*k->angular_velocity) * cfg.dt_substeps(), alpha);
                }
                if (k->pitch) {
                    rb.avatar_controller().increment_pitch((*k->angular_velocity) * cfg.dt_substeps(), alpha);
                }
            }
        }
    }
    if (enable_controls) {
        for (const auto& k : avatar_controller_idle_bindings_) {
            auto& rb = get_rigid_body_vehicle(*k->node);
            rb.avatar_controller().apply();
        }
    }
    // Vehicle controller
    if (enable_controls) {
        for (const auto& k : car_controller_idle_bindings_) {
            auto& rb = get_rigid_body_vehicle(*k->node);
            rb.vehicle_controller().reset_parameters(
                k->surface_power,
                k->steer_angle);
            rb.vehicle_controller().reset_relaxation(
                k->drive_relaxation,
                k->steer_relaxation);
        }
    }
    for (auto& k : car_controller_key_bindings_) {
        float alpha = get_alpha(
            k->button_press,
            nullptr,
            nullptr,
            &k->gamepad_analog_axes_position,
            0.f,
            1.f,
            cfg);
        if (enable_controls && !std::isnan(alpha)) {
            auto& rb = get_rigid_body_vehicle(*k->node);
            if (k->surface_power.has_value()) {
                rb.vehicle_controller().drive(*k->surface_power, alpha);
            }
            if (k->steer_left_amount.has_value()) {
                rb.vehicle_controller().set_stearing_wheel_amount(*k->steer_left_amount, alpha);
            }
            if (k->ascend_velocity.has_value()) {
                rb.vehicle_controller().ascend_by((*k->ascend_velocity) * cfg.dt_substeps());
            }
        }
    }
    if (enable_controls) {
        for (const auto& k : car_controller_idle_bindings_) {
            auto& rb = get_rigid_body_vehicle(*k->node);
            rb.vehicle_controller().apply();
        }
    }
    // Plane controller
    if (enable_controls) {
        for (const auto& k : plane_controller_idle_bindings_) {
            if (k->player->scene_node_scheduled_for_deletion()) {
                continue;
            }
            auto& rb = k->player->rigid_body();
            rb.plane_controller().reset_parameters(0.f, 0.f, 0.f, 0.f, 0.f);
            rb.plane_controller().reset_relaxation(0.f, 0.f, 0.f, 0.f);
        }
    }
    for (auto& k : plane_controller_key_bindings_) {
        if (k->player->scene_node_scheduled_for_deletion()) {
            continue;
        }
        float alpha = get_alpha(
            k->button_press,
            k->cursor_movement.get(),
            nullptr,
            &k->gamepad_analog_axes_position,
            0.f,
            1.f,
            cfg);
        if (enable_controls && !std::isnan(alpha)) {
            auto& rb = k->player->rigid_body();
            if (k->turbine_power.has_value()) {
                rb.plane_controller().accelerate(*k->turbine_power, alpha);
            }
            if (k->brake.has_value()) {
                rb.plane_controller().brake(*k->brake, alpha);
            }
            if (k->pitch.has_value()) {
                rb.plane_controller().pitch(alpha * (*k->pitch), alpha);
            }
            if (k->yaw.has_value()) {
                rb.plane_controller().yaw(alpha * (*k->yaw), alpha);
            }
            if (k->roll.has_value()) {
                rb.plane_controller().roll(alpha * (*k->roll), alpha);
            }
        }
    }
    if (enable_controls) {
        for (const auto& k : plane_controller_idle_bindings_) {
            if (k->player->scene_node_scheduled_for_deletion()) {
                continue;
            }
            auto& rb = k->player->rigid_body();
            rb.plane_controller().apply();
        }
    }
    // Weapon inventory
    for (auto& k : weapon_cycle_key_bindings_) {
        if (k->player->scene_node_scheduled_for_deletion()) {
            continue;
        }
        float alpha = get_alpha(
            k->button_press,
            nullptr,
            k->scroll_wheel_movement.get(),
            nullptr,
            0.f,
            1.f,
            cfg);
        if (enable_controls && !std::isnan(alpha)) {
            auto& wc = k->player->weapon_cycle();
            if (k->direction == 1) {
                wc.equip_next_weapon(k->player->id());
            } else if (k->direction == -1) {
                wc.equip_previous_weapon(k->player->id());
            } else {
                THROW_OR_ABORT("Weapon cycle direction not -1 or 1");
            }
        }
    }
    // Gun
    for (auto& k : gun_key_bindings_) {
        if (k->player->scene_node_scheduled_for_deletion()) {
            continue;
        }
        if (k->button_press.keys_down() && enable_controls) {
            k->player->trigger_gun();
        }
    }
    // Player
    for (auto& k : player_key_bindings_) {
        if (k->player->scene_node_scheduled_for_deletion()) {
            continue;
        }
        if (k->button_press.keys_pressed() && enable_controls) {
            if (k->select_next_opponent) {
                k->player->select_opponent(OpponentSelectionStrategy::NEXT);
            }
            if (k->select_next_vehicle) {
                k->player->select_next_vehicle();
            }
            if (k->reset_vehicle) {
                k->player->request_reset_vehicle_to_last_checkpoint();
            }
        }
    }
}

std::optional<RenderSetup> KeyBindings::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return std::nullopt;
}

void KeyBindings::render_without_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("KeyBindings::render");
    // Near camera
    for (auto& k : camera_key_bindings_) {
        if (k->tpe == CameraCycleType::FAR) {
            continue;
        }
        if (k->button_press.keys_pressed()) {
            selected_cameras_.cycle_camera(k->tpe);
        }
    }
}

void KeyBindings::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "KeyBindings\n";
}
