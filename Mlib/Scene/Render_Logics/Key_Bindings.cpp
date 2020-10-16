#include "Key_Bindings.hpp"
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Physics/Objects/Gun.hpp>
#include <Mlib/Physics/Objects/Relative_Transformer.hpp>
#include <Mlib/Physics/Objects/Rigid_Body.hpp>
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

void KeyBindings::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{}

float KeyBindings::near_plane() const {
    throw std::runtime_error("KeyBindings::requires_postprocessing not implemented");
}

float KeyBindings::far_plane() const {
    throw std::runtime_error("KeyBindings::requires_postprocessing not implemented");
}

const FixedArray<float, 4, 4>& KeyBindings::vp() const {
    throw std::runtime_error("KeyBindings::vp not implemented");
}

const FixedArray<float, 4, 4>& KeyBindings::iv() const {
    throw std::runtime_error("KeyBindings::iv not implemented");
}

bool KeyBindings::requires_postprocessing() const {
    throw std::runtime_error("KeyBindings::requires_postprocessing not implemented");
}

void KeyBindings::increment_external_forces(const std::list<std::shared_ptr<RigidBody>>& olist, bool burn_in) {
    if (!burn_in && !focus_.empty() && (focus_.back() == Focus::SCENE)) {
        // if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        //     glfwSetWindowShouldClose(window_, GLFW_TRUE);
        // }
        if (print_gamepad_buttons_) {
            button_press_.print();
        }
        // std::cerr << std::endl;
        // std::cerr << std::endl;
        // for(size_t i = 0; i < 15; ++i) {
        //     std::cerr << i << "=" << (uint)gamepad_state.buttons[i] << " ";
        // }
        // std::cerr << std::endl;
        // for(size_t i = 0; i < 6; ++i) {
        //     std::cerr << i << "=" << gamepad_state.axes[i] << " ";
        // }
        // std::cerr << std::endl;
        for(const auto& k : camera_key_bindings_) {
            if (button_press_.key_pressed(k.base)) {
                auto& cams = selected_cameras_.camera_cycle_near;
                if (cams.empty()) {
                    throw std::runtime_error("Near camera cycle is empty");
                }
                auto it = std::find(cams.begin(), cams.end(), selected_cameras_.camera_node_name);
                if (it == cams.end() || ++it == cams.end()) {
                    it = cams.begin();
                }
                selected_cameras_.camera_node_name = *it;
            }
        }
        for(const auto& k : absolute_movable_idle_bindings_) {
            auto m = scene_.get_node(k.node)->get_absolute_movable();
            auto rb = dynamic_cast<RigidBody*>(m);
            if (rb == nullptr) {
                throw std::runtime_error("Absolute movable is not a rigid body");
            }
            rb->set_surface_power("main", 0);
            rb->set_surface_power("breaks", 0);
            rb->set_max_velocity(INFINITY);
            for(auto& t : rb->tires_) {
                t.second.angle = 0;
            }
            rb->tires_z_ = k.tires_z;
        }
        for(const auto& k : absolute_movable_key_bindings_) {
            float alpha = button_press_.key_alpha(k.base_key, 0.05);
            if (!std::isnan(alpha)) {
                auto m = scene_.get_node(k.node)->get_absolute_movable();
                auto rb = dynamic_cast<RigidBody*>(m);
                if (rb == nullptr) {
                    throw std::runtime_error("Absolute movable is not a rigid body");
                }
                rb->integrate_force(rb->abs_F(k.force));
                if (any(k.rotate != 0.f)) {
                    rb->rbi_.rotation_ = dot2d(rb->rbi_.rotation_, rodrigues(alpha * k.rotate));
                }
                rb->set_surface_power("main", k.surface_power);
                rb->set_surface_power("breaks", k.surface_power);
                rb->set_max_velocity(k.max_velocity);
                if (k.tire_id != SIZE_MAX) {
                    rb->set_tire_angle(k.tire_id, alpha * M_PI / 180.f * k.tire_angle_interp(std::sqrt(sum(squared(rb->rbi_.v_))) * 3.6f));
                }
                rb->tires_z_ += k.tires_z;
            }
        }
        for(const auto& k : absolute_movable_idle_bindings_) {
            auto m = scene_.get_node(k.node)->get_absolute_movable();
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
        for(const auto& k : relative_movable_key_bindings_) {
            auto m = scene_.get_node(k.node)->get_relative_movable();
            auto rt = dynamic_cast<RelativeTransformer*>(m);
            if (rt == nullptr) {
                throw std::runtime_error("Relative movable is not a relative transformer");
            }
            rt->w_ = 0.f;
        }
        for(const auto& k : relative_movable_key_bindings_) {
            float alpha = button_press_.key_alpha(k.base_key);
            if (!std::isnan(alpha)) {
                auto m = scene_.get_node(k.node)->get_relative_movable();
                auto rt = dynamic_cast<RelativeTransformer*>(m);
                if (rt == nullptr) {
                    throw std::runtime_error("Relative movable is not a relative transformer");
                }
                rt->w_ = (1 - alpha) * k.angular_velocity_press + alpha * k.angular_velocity_repeat;
            }
        }
        for(const auto& k : gun_key_bindings_) {
            if (button_press_.key_down(k.base)) {
                auto m = scene_.get_node(k.node)->get_absolute_observer();
                auto gun = dynamic_cast<Gun*>(m);
                if (gun == nullptr) {
                    throw std::runtime_error("Absolute observer is not a gun");
                }
                gun->trigger();
            }
        }
    }
}
