#include "Key_Bindings.hpp"
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Physics/Advance_Times/Gun.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Pitch_Look_At_Node.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Relative_Transformer.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Physics/Gravity.hpp>
#include <Mlib/Physics/Misc/Weapon_Cycle.hpp>
#include <Mlib/Physics/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Rigid_Body_Avatar_Controller.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Rigid_Body_Plane_Controller.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Rigid_Body_Vehicle_Controller.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Render/Key_Bindings/Absolute_Movable_Idle_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Absolute_Movable_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Avatar_Controller_Idle_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Avatar_Controller_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Camera_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Car_Controller_Idle_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Car_Controller_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Gun_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Plane_Controller_Idle_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Plane_Controller_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Player_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Relative_Movable_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Weapon_Cycle_Key_Binding.hpp>
#include <Mlib/Render/Selected_Cameras.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Render/Ui/Cursor_States.hpp>
#include <Mlib/Scene_Graph/Animation_State_Updater.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>

using namespace Mlib;

KeyBindings::KeyBindings(
    ButtonPress& button_press,
    bool print_gamepad_buttons,
    SelectedCameras& selected_cameras,
    const Focuses& focuses)
: button_press_{button_press},
  print_gamepad_buttons_{print_gamepad_buttons},
  selected_cameras_{selected_cameras},
  focuses_{focuses}
{}

KeyBindings::~KeyBindings() {
    std::set<SceneNode*> nodes;
    for (auto& b : absolute_movable_idle_bindings_) { nodes.insert(b.node); }
    for (auto& b : absolute_movable_key_bindings_) { nodes.insert(b.node); }
    for (auto& b : relative_movable_key_bindings_) { nodes.insert(b.node); }
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
        node->remove_destruction_observer(this);
    }
}

void KeyBindings::notify_destroyed(void* destroyed_object) {
    absolute_movable_idle_bindings_.remove_if([destroyed_object](const auto& b){return b.node == destroyed_object;});
    absolute_movable_key_bindings_.remove_if([destroyed_object](const auto& b){return b.node == destroyed_object;});
    relative_movable_key_bindings_.remove_if([destroyed_object](const auto& b){return b.node == destroyed_object;});
    car_controller_idle_bindings_.remove_if([destroyed_object](const auto& b){return b.node == destroyed_object;});
    car_controller_key_bindings_.remove_if([destroyed_object](const auto& b){return b.node == destroyed_object;});
    plane_controller_idle_bindings_.remove_if([destroyed_object](const auto& b){return b.node == destroyed_object;});
    plane_controller_key_bindings_.remove_if([destroyed_object](const auto& b){return b.node == destroyed_object;});
    avatar_controller_idle_bindings_.remove_if([destroyed_object](const auto& b){return b.node == destroyed_object;});
    avatar_controller_key_bindings_.remove_if([destroyed_object](const auto& b){return b.node == destroyed_object;});
    weapon_cycle_key_bindings_.remove_if([destroyed_object](const auto& b){return b.node == destroyed_object;});
    gun_key_bindings_.remove_if([destroyed_object](const auto& b){return b.node == destroyed_object;});
    player_key_bindings_.remove_if([destroyed_object](const auto& b){return b.node == destroyed_object;});
}

void KeyBindings::add_camera_key_binding(const CameraKeyBinding& b) {
    camera_key_bindings_.push_back(b);
}

void KeyBindings::add_absolute_movable_idle_binding(const AbsoluteMovableIdleBinding& b) {
    b.node->add_destruction_observer(this, true);
    absolute_movable_idle_bindings_.push_back(b);
}

void KeyBindings::add_absolute_movable_key_binding(const AbsoluteMovableKeyBinding& b) {
    b.node->add_destruction_observer(this, true);
    absolute_movable_key_bindings_.push_back(b);
}

void KeyBindings::add_relative_movable_key_binding(const RelativeMovableKeyBinding& b) {
    b.node->add_destruction_observer(this, true);
    relative_movable_key_bindings_.push_back(b);
}

const CarControllerIdleBinding& KeyBindings::add_car_controller_idle_binding(const CarControllerIdleBinding& b) {
    b.node->add_destruction_observer(this, true);
    car_controller_idle_bindings_.push_back(b);
    return car_controller_idle_bindings_.back();
}

const CarControllerKeyBinding& KeyBindings::add_car_controller_key_binding(const CarControllerKeyBinding& b) {
    b.node->add_destruction_observer(this, true);
    car_controller_key_bindings_.push_back(b);
    return car_controller_key_bindings_.back();
}

const PlaneControllerIdleBinding& KeyBindings::add_plane_controller_idle_binding(const PlaneControllerIdleBinding& b) {
    b.node->add_destruction_observer(this, true);
    plane_controller_idle_bindings_.push_back(b);
    return plane_controller_idle_bindings_.back();
}

const PlaneControllerKeyBinding& KeyBindings::add_plane_controller_key_binding(const PlaneControllerKeyBinding& b) {
    b.node->add_destruction_observer(this, true);
    plane_controller_key_bindings_.push_back(b);
    return plane_controller_key_bindings_.back();
}

void KeyBindings::add_avatar_controller_idle_binding(const AvatarControllerIdleBinding& b) {
    b.node->add_destruction_observer(this, true);
    avatar_controller_idle_bindings_.push_back(b);
}

void KeyBindings::add_avatar_controller_key_binding(const AvatarControllerKeyBinding& b) {
    b.node->add_destruction_observer(this, true);
    avatar_controller_key_bindings_.push_back(b);
}

void KeyBindings::add_weapon_inventory_key_binding(const WeaponCycleKeyBinding& b) {
    b.node->add_destruction_observer(this, true);
    weapon_cycle_key_bindings_.push_back(b);
}

const GunKeyBinding& KeyBindings::add_gun_key_binding(const GunKeyBinding& b) {
    b.node->add_destruction_observer(this, true);
    gun_key_bindings_.push_back(b);
    return gun_key_bindings_.back();
}

const PlayerKeyBinding& KeyBindings::add_player_key_binding(const PlayerKeyBinding& b) {
    b.node->add_destruction_observer(this, true);
    player_key_bindings_.push_back(b);
    return player_key_bindings_.back();
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

void KeyBindings::delete_gun_key_binding(const GunKeyBinding& deleted_key_binding) {
    gun_key_bindings_.remove_if([&deleted_key_binding](const auto& b){return &b == &deleted_key_binding;});
}

void KeyBindings::delete_player_key_binding(const PlayerKeyBinding& deleted_key_binding) {
    player_key_bindings_.remove_if([&deleted_key_binding](const auto& b){return &b == &deleted_key_binding;});
}

void KeyBindings::increment_external_forces(
    const std::list<std::shared_ptr<RigidBodyVehicle>>& olist,
    bool burn_in,
    const PhysicsEngineConfig& cfg)
{
    if (burn_in) {
        return;
    }
    {
        std::lock_guard lock{focuses_.mutex};
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

    // Camera
    for (const auto& k : camera_key_bindings_) {
        if (button_press_.key_pressed(k.base)) {
            auto& cams = selected_cameras_.camera_cycle_near;
            if (cams.empty()) {
                throw std::runtime_error("Near camera cycle is empty");
            }
            auto it = std::find(cams.begin(), cams.end(), selected_cameras_.camera_node_name());
            if (it == cams.end() || ++it == cams.end()) {
                it = cams.begin();
            }
            selected_cameras_.set_camera_node_name(*it);
        }
    }
    // Absolute movable
    for (const auto& k : absolute_movable_idle_bindings_) {
        auto rb = dynamic_cast<RigidBodyVehicle*>(&k.node->get_absolute_movable());
        if (rb == nullptr) {
            throw std::runtime_error("Absolute movable is not a rigid body");
        }
        rb->set_surface_power("main", 0);
        rb->set_surface_power("brakes", 0);
        rb->set_max_velocity(INFINITY);
        for (auto& t : rb->tires_) {
            t.second.angle_y = 0;
            // t.second.accel_x = 0;
        }
        rb->tires_z_ = k.tires_z;
    }
    for (const auto& k : absolute_movable_key_bindings_) {
        float alpha = button_press_.key_alpha(k.base_key, 0.05f);
        if (!std::isnan(alpha)) {
            auto rb = dynamic_cast<RigidBodyVehicle*>(&k.node->get_absolute_movable());
            if (rb == nullptr) {
                throw std::runtime_error("Absolute movable is not a rigid body");
            }
            if (any(k.force.vector != 0.f)) {
                rb->integrate_force(rb->abs_F(k.force), cfg);
            }
            if (any(k.rotate != 0.f)) {
                rb->rbi_.rbp_.rotation_ = dot2d(rb->rbi_.rbp_.rotation_, rodrigues1(alpha * k.rotate));
            }
            if (k.car_surface_power.has_value()) {
                rb->set_surface_power("main", k.car_surface_power.value());
                rb->set_surface_power("brakes", k.car_surface_power.value());
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
            throw std::runtime_error("Absolute movable is not a rigid body");
        }
        if (any(abs(rb->tires_z_) > float(1e-12))) {
            rb->tires_z_ /= std::sqrt(sum(squared(rb->tires_z_)));
        } else {
            rb->tires_z_ = { 0.f, 0.f, 1.f };
            rb->set_surface_power("main", NAN);
            rb->set_surface_power("brakes", NAN);
        }
        if (rb->animation_state_updater_ != nullptr) {
            rb->animation_state_updater_->notify_movement_intent();
        }
    }
    // Relative movable
    for (const auto& k : relative_movable_key_bindings_) {
        auto& m = k.node->get_relative_movable();
        auto rt = dynamic_cast<RelativeTransformer*>(&m);
        auto ypln = dynamic_cast<YawPitchLookAtNodes*>(&m);
        if (rt != nullptr) {
            rt->w_ = 0.f;
        } else if (ypln != nullptr) {
            // Do nothing (yet)
        } else {
            throw std::runtime_error("Relative movable is neither a relative transformer nor yaw/pitch-look-at-nodes");
        }
    }
    for (auto& k : relative_movable_key_bindings_) {
        auto& m = k.node->get_relative_movable();
        auto rt = dynamic_cast<RelativeTransformer*>(&m);
        auto ypln = dynamic_cast<YawPitchLookAtNodes*>(&m);
        float alpha = button_press_.keys_alpha(k.base_combo);
        if (!std::isnan(alpha)) {
            float w = ((1 - alpha) * k.angular_velocity_press + alpha * k.angular_velocity_repeat);
            if (rt != nullptr) {
                rt->w_ = w * k.rotation_axis;
            } else if (ypln != nullptr) {
                if (all(k.rotation_axis == FixedArray<float, 3>{0, 1, 0})) {
                    ypln->increment_yaw(w * cfg.dt);
                } else if (all(k.rotation_axis == FixedArray<float, 3>{1, 0, 0})) {
                    ypln->pitch_look_at_node()->increment_pitch(w * cfg.dt);
                } else {
                    throw std::runtime_error("Unsupported rotation axis for yaw/pitch-look-at-nodes");
                }
            } else {
                throw std::runtime_error("Relative movable is neither a relative transformer nor yaw/pitch-look-at-nodes");
            }
        }
        if (k.cursor_movement != nullptr) {
            float beta = k.cursor_movement->axis_alpha(k.base_cursor_axis);
            if (!std::isnan(beta)) {
                float dangle = beta * k.speed_cursor;
                if (rt != nullptr) {
                    // rt->w_ = beta * k.angular_velocity_repeat;
                    rt->transformation_matrix_.R() = dot2d(
                        rodrigues2(k.rotation_axis, dangle),
                        rt->transformation_matrix_.R());
                } else if (ypln != nullptr) {
                    if (all(k.rotation_axis == FixedArray<float, 3>{0, 1, 0})) {
                        ypln->increment_yaw(dangle);
                    } else if (all(k.rotation_axis == FixedArray<float, 3>{1, 0, 0})) {
                        ypln->pitch_look_at_node()->increment_pitch(dangle);
                    } else {
                        throw std::runtime_error("Unsupported rotation axis for yaw/pitch-look-at-nodes");
                    }
                } else {
                    throw std::runtime_error("Relative movable is neither a relative transformer nor yaw/pitch-look-at-nodes");
                }
            }
        }
    }
    // Avatar controller
    for (const auto& k : avatar_controller_idle_bindings_) {
        auto rb = dynamic_cast<RigidBodyVehicle*>(&k.node->get_absolute_movable());
        if (rb == nullptr) {
            throw std::runtime_error("Absolute movable is not a rigid body");
        }
        rb->avatar_controller().reset();
    }
    for (const auto& k : avatar_controller_key_bindings_) {
        auto rb = dynamic_cast<RigidBodyVehicle*>(&k.node->get_absolute_movable());
        if (rb == nullptr) {
            throw std::runtime_error("Absolute movable is not a rigid body");
        }
        float alpha = button_press_.key_alpha(k.base_key, 0.05f);
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
        if (k.cursor_movement != nullptr) {
            float beta = k.cursor_movement->axis_alpha(k.base_cursor_axis);
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
            throw std::runtime_error("Absolute movable is not a rigid body");
        }
        rb->avatar_controller().apply();
    }
    // Vehicle controller
    for (const auto& k : car_controller_idle_bindings_) {
        auto rb = dynamic_cast<RigidBodyVehicle*>(&k.node->get_absolute_movable());
        if (rb == nullptr) {
            throw std::runtime_error("Absolute movable is not a rigid body");
        }
        rb->vehicle_controller().reset(
            k.surface_power,
            k.steer_angle);
    }
    for (const auto& k : car_controller_key_bindings_) {
        float alpha = button_press_.keys_alpha(k.base_combo, 0.05f);
        if (!std::isnan(alpha)) {
            auto rb = dynamic_cast<RigidBodyVehicle*>(&k.node->get_absolute_movable());
            if (rb == nullptr) {
                throw std::runtime_error("Absolute movable is not a rigid body");
            }
            if (k.surface_power.has_value()) {
                rb->vehicle_controller().drive(k.surface_power.value());
            }
            if (k.tire_angle_interp.has_value()) {
                float v = std::abs(dot0d(
                    rb->rbi_.rbp_.v_,
                    rb->rbi_.rbp_.rotation_.column(2)));
                rb->vehicle_controller().steer(alpha * degrees * k.tire_angle_interp.value()(v * 3.6f));
            }
            if (k.ascend_velocity.has_value()) {
                rb->vehicle_controller().ascend_by(k.ascend_velocity.value() * cfg.dt);
            }
        }
    }
    for (const auto& k : car_controller_idle_bindings_) {
        auto rb = dynamic_cast<RigidBodyVehicle*>(&k.node->get_absolute_movable());
        if (rb == nullptr) {
            throw std::runtime_error("Absolute movable is not a rigid body");
        }
        rb->vehicle_controller().apply();
    }
    // Plane controller
    for (const auto& k : plane_controller_idle_bindings_) {
        auto rb = dynamic_cast<RigidBodyVehicle*>(&k.node->get_absolute_movable());
        if (rb == nullptr) {
            throw std::runtime_error("Absolute movable is not a rigid body");
        }
        rb->plane_controller().reset(0.f, 0.f, 0.f, 0.f, 0.f);
    }
    for (const auto& k : plane_controller_key_bindings_) {
        float alpha = button_press_.keys_alpha(k.base_combo, 0.05f);
        if (!std::isnan(alpha)) {
            auto rb = dynamic_cast<RigidBodyVehicle*>(&k.node->get_absolute_movable());
            if (rb == nullptr) {
                throw std::runtime_error("Absolute movable is not a rigid body");
            }
            if (k.turbine_power.has_value()) {
                rb->plane_controller().accelerate(k.turbine_power.value());
            }
            if (k.brake.has_value()) {
                rb->plane_controller().brake(k.brake.value());
            }
            if (k.pitch.has_value()) {
                rb->plane_controller().pitch(alpha * k.pitch.value());
            }
            if (k.yaw.has_value()) {
                rb->plane_controller().yaw(alpha * k.yaw.value());
            }
            if (k.roll.has_value()) {
                rb->plane_controller().roll(alpha * k.roll.value());
            }
        }
    }
    for (const auto& k : plane_controller_idle_bindings_) {
        auto rb = dynamic_cast<RigidBodyVehicle*>(&k.node->get_absolute_movable());
        if (rb == nullptr) {
            throw std::runtime_error("Absolute movable is not a rigid body");
        }
        rb->plane_controller().apply();
    }
    // Weapon inventory
    for (auto& k : weapon_cycle_key_bindings_) {
        float beta = k.scroll_wheel_movement->axis_alpha(k.base_scroll_wheel_axis);
        if (!std::isnan(beta)) {
            auto wc = dynamic_cast<WeaponCycle*>(&k.node->get_node_modifier());
            if (wc == nullptr) {
                throw std::runtime_error("Node modifier is not a weapon cycle");
            }
            if (k.direction == 1) {
                wc->equip_next_weapon();
            } else if (k.direction == -1) {
                wc->equip_previous_weapon();
            } else {
                throw std::runtime_error("Weapon cycle direction not -1 or 1");
            }
        }
    }
    // Gun
    for (const auto& k : gun_key_bindings_) {
        if (button_press_.keys_down(k.base_combo)) {
            auto gun = dynamic_cast<Gun*>(&k.node->get_absolute_observer());
            if (gun == nullptr) {
                throw std::runtime_error("Absolute observer is not a gun");
            }
            gun->trigger();
        }
    }
    // Player
    for (const auto& k : player_key_bindings_) {
        float alpha = button_press_.keys_alpha(k.base_combo, 0.05f);
        if (!std::isnan(alpha) && alpha == 0) {
            auto rb = dynamic_cast<RigidBodyVehicle*>(&k.node->get_absolute_movable());
            if (rb == nullptr) {
                throw std::runtime_error("Absolute movable is not a rigid body");
            }
            if (rb->driver_ == nullptr) {
                throw std::runtime_error("Rigid body has no driver");
            }
            Player* player = dynamic_cast<Player*>(rb->driver_);
            if (player == nullptr) {
                throw std::runtime_error("Driver is not a player");
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
