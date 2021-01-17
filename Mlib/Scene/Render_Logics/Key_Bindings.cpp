#include "Key_Bindings.hpp"
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Physics/Advance_Times/Gun.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Relative_Transformer.hpp>
#include <Mlib/Physics/Misc/Rigid_Body.hpp>
#include <Mlib/Render/Selected_Cameras.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>

using namespace Mlib;

KeyBindings::KeyBindings(
    ButtonPress& button_press,
    bool print_gamepad_buttons,
    SelectedCameras& selected_cameras,
    const std::list<Focus>& focus,
    const Scene& scene)
: button_press_{button_press},
  print_gamepad_buttons_{print_gamepad_buttons},
  scene_{scene},
  selected_cameras_{selected_cameras},
  focus_{focus}
{}

KeyBindings::~KeyBindings() {
    std::set<SceneNode*> nodes;
    for (auto& b : absolute_movable_idle_bindings_) { nodes.insert(b.node); }
    for (auto& b : absolute_movable_key_bindings_) { nodes.insert(b.node); }
    for (auto& b : relative_movable_key_bindings_) { nodes.insert(b.node); }
    for (auto& b : gun_key_bindings_) { nodes.insert(b.node); }
    for (auto& node : nodes) {
        node->remove_destruction_observer(this);
    }
}

void KeyBindings::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{}

void KeyBindings::notify_destroyed(void* destroyed_object) {
    absolute_movable_idle_bindings_.remove_if([destroyed_object](const auto& b){return b.node == destroyed_object;});
    absolute_movable_key_bindings_.remove_if([destroyed_object](const auto& b){return b.node == destroyed_object;});
    relative_movable_key_bindings_.remove_if([destroyed_object](const auto& b){return b.node == destroyed_object;});
    gun_key_bindings_.remove_if([this, destroyed_object](const auto& b){return b.node == destroyed_object;});
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

void KeyBindings::add_gun_key_binding(const GunKeyBinding& b) {
    b.node->add_destruction_observer(this, true);
    gun_key_bindings_.push_back(b);
}

void KeyBindings::increment_external_forces(const std::list<std::shared_ptr<RigidBody>>& olist, bool burn_in, const PhysicsEngineConfig& cfg) {
    if (!burn_in && !focus_.empty() && (focus_.back() == Focus::SCENE)) {
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
        for (const auto& k : absolute_movable_idle_bindings_) {
            auto m = k.node->get_absolute_movable();
            auto rb = dynamic_cast<RigidBody*>(m);
            if (rb == nullptr) {
                throw std::runtime_error("Absolute movable is not a rigid body");
            }
            rb->set_surface_power("main", 0);
            rb->set_surface_power("breaks", 0);
            rb->set_max_velocity(INFINITY);
            for (auto& t : rb->tires_) {
                t.second.angle_y = 0;
                // t.second.accel_x = 0;
            }
            rb->tires_z_ = k.tires_z;
        }
        for (const auto& k : absolute_movable_key_bindings_) {
            float alpha = button_press_.key_alpha(k.base_key, 0.05);
            if (!std::isnan(alpha)) {
                auto m = k.node->get_absolute_movable();
                auto rb = dynamic_cast<RigidBody*>(m);
                if (rb == nullptr) {
                    throw std::runtime_error("Absolute movable is not a rigid body");
                }
                if (cfg.resolve_collision_type == ResolveCollisionType::PENALTY) {
                    rb->integrate_force(rb->abs_F(k.force));
                } else if (cfg.resolve_collision_type == ResolveCollisionType::SEQUENTIAL_PULSES) {
                    rb->rbi_.rbp_.integrate_impulse(rb->abs_F({
                        vector: k.force.vector * (cfg.dt / cfg.oversampling),
                        position: k.force.position}));
                } else {
                    throw std::runtime_error("Unknown resolve collision type in key_bindings");
                }
                if (any(k.rotate != 0.f)) {
                    rb->rbi_.rbp_.rotation_ = dot2d(rb->rbi_.rbp_.rotation_, rodrigues(alpha * k.rotate));
                }
                rb->set_surface_power("main", k.surface_power);
                rb->set_surface_power("breaks", k.surface_power);
                rb->set_max_velocity(k.max_velocity);
                if (k.tire_id != SIZE_MAX) {
                    if (false) {
                        float a = 9.8 * 1;
                        float l = 2.55;
                        float r = sum(squared(rb->rbi_.rbp_.v_)) / a;
                        float angle = std::asin(std::clamp(l / r, 0.f, 1.f));
                        rb->set_tire_angle_y(k.tire_id, angle * alpha * sign(k.tire_angle_interp(0)));
                    }
                    // float v = std::sqrt(sum(squared(rb->rbi_.rbp_.v_)));
                    float v = std::abs(dot0d(
                        rb->rbi_.rbp_.v_,
                        rb->rbi_.rbp_.rotation_.column(2)));
                    rb->set_tire_angle_y(k.tire_id, alpha * M_PI / 180.f * k.tire_angle_interp(v * 3.6f));
                    // rb->set_tire_accel_x(k.tire_id, alpha * sign(k.tire_angle_interp(0)));
                }
                rb->tires_z_ += k.tires_z;
            }
        }
        for (const auto& k : absolute_movable_idle_bindings_) {
            auto m = k.node->get_absolute_movable();
            auto rb = dynamic_cast<RigidBody*>(m);
            if (rb == nullptr) {
                throw std::runtime_error("Absolute movable is not a rigid body");
            }
            if (any(abs(rb->tires_z_) > float(1e-12))) {
                rb->tires_z_ /= std::sqrt(sum(squared(rb->tires_z_)));
            } else {
                rb->tires_z_ = {0, 0, 1};
                rb->set_surface_power("main", NAN);
                rb->set_surface_power("breaks", NAN);
            }
        }
        for (const auto& k : relative_movable_key_bindings_) {
            auto m = k.node->get_relative_movable();
            auto rt = dynamic_cast<RelativeTransformer*>(m);
            if (rt == nullptr) {
                throw std::runtime_error("Relative movable is not a relative transformer");
            }
            rt->w_ = 0.f;
        }
        for (const auto& k : relative_movable_key_bindings_) {
            float alpha = button_press_.key_alpha(k.base_key);
            if (!std::isnan(alpha)) {
                auto m = k.node->get_relative_movable();
                auto rt = dynamic_cast<RelativeTransformer*>(m);
                if (rt == nullptr) {
                    throw std::runtime_error("Relative movable is not a relative transformer");
                }
                rt->w_ = (1 - alpha) * k.angular_velocity_press + alpha * k.angular_velocity_repeat;
            }
        }
        for (const auto& k : gun_key_bindings_) {
            if (button_press_.key_down(k.base)) {
                auto m = k.node->get_absolute_observer();
                auto gun = dynamic_cast<Gun*>(m);
                if (gun == nullptr) {
                    throw std::runtime_error("Absolute observer is not a gun");
                }
                gun->trigger();
            }
        }
    }
}
