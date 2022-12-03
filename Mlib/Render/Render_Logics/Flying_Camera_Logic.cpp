#ifdef __ANDROID__
#else
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#endif

#include "Flying_Camera_Logic.hpp"
#include <Mlib/Fps/Set_Fps.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Selected_Cameras.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Render/Ui/Cursor_States.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>
#include <Mlib/Threads/Termination_Manager.hpp>

using namespace Mlib;

static void flying_key_callback(
#ifndef __ANDROID__
    GLFWwindow* window,
#endif
    ButtonPress& button_press,
    FlyingCameraUserClass& user_object)
{
#ifndef __ANDROID__
    if (button_press.key_pressed({.key = "ESCAPE"})) {
        GLFW_CHK(glfwSetWindowShouldClose(window, GLFW_TRUE));
    }
#endif
    if (button_press.key_down({.key = "LEFT_CONTROL"})) {
        if (button_press.key_down({.key = "UP"})) {
            user_object.obj_angles(2) += 0.01f;
        }
        if (button_press.key_down({.key = "DOWN"})) {
            user_object.obj_angles(2) -= 0.01f;
        }
        if (button_press.key_down({.key = "LEFT"})) {
            user_object.obj_angles(1) += 0.01f;
        }
        if (button_press.key_down({.key = "RIGHT"})) {
            user_object.obj_angles(1) -= 0.01f;
        }
        if (button_press.key_down({.key = "PAGE_UP"})) {
            user_object.obj_angles(0) += 0.01f;
        }
        if (button_press.key_down({.key = "PAGE_DOWN"})) {
            user_object.obj_angles(0) -= 0.01f;
        }
        if (button_press.key_down({.key = "KP_ADD"})) {
            user_object.obj_position(1) += 0.04f;
        }
        if (button_press.key_down({.key = "KP_SUBTRACT"})) {
            user_object.obj_position(1) -= 0.04f;
        }
    } else {
        if (button_press.key_down({.key = "UP"})) {
            user_object.position(2) -= 0.04f;
        }
        if (button_press.key_down({.key = "DOWN"})) {
            user_object.position(2) += 0.04f;
        }
        if (button_press.key_down({.key = "LEFT"})) {
            user_object.angles(1) += 0.01f;
        }
        if (button_press.key_down({.key = "RIGHT"})) {
            user_object.angles(1) -= 0.01f;
        }
        if (button_press.key_down({.key = "PAGE_UP"})) {
            user_object.angles(0) += 0.01f;
        }
        if (button_press.key_down({.key = "PAGE_DOWN"})) {
            user_object.angles(0) -= 0.01f;
        }
        if (button_press.key_down({.key = "KP_ADD"})) {
            user_object.position(1) += 0.04f;
        }
        if (button_press.key_down({.key = "KP_SUBTRACT"})) {
            user_object.position(1) -= 0.04f;
        }
    }
}

static void nofly_key_callback(
    ButtonPress& button_press,
    FlyingCameraUserClass& user_object)
{
    // if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    //     GLFW_CHK(glfwSetWindowShouldClose(window, GLFW_TRUE));
    // }
    if (button_press.key_pressed({.key = "L"})) {
        user_object.cameras.cycle_far_camera();
    }
    // if (button_press.key_pressed({.key = "P"})) {
    //     if (user_object.physics_set_fps != nullptr) {
    //         user_object.physics_set_fps->toggle_pause_resume();
    //     }
    // }
    if (button_press.key_down({.key = "LEFT_CONTROL"})) {
        if (button_press.key_pressed({.key = "W"})) {
            user_object.wire_frame = zapped(user_object.wire_frame);
        }
        if (button_press.key_pressed({.key = "D"})) {
            user_object.depth_test = zapped(user_object.depth_test);
        }
        if (button_press.key_pressed({.key = "C"})) {
            user_object.cull_faces = zapped(user_object.cull_faces);
        }
    }
}

FlyingCameraLogic::FlyingCameraLogic(
#ifndef __ANDROID__
    GLFWwindow* window,
#endif
    const ButtonStates& button_states,
    const Scene& scene,
    FlyingCameraUserClass& user_object,
    bool fly,
    bool rotate)
: scene_{scene},
  user_object_{user_object},
  button_press_{button_states},
  fly_{fly},
  rotate_{rotate}
#ifndef __ANDROID__
  ,window_{window}
#endif
{
    // GLFW_CHK(glfwGetWindowPos(window, &user_object_.windowed_x, &user_object_.windowed_y));
    // GLFW_CHK(glfwGetWindowSize(window, &user_object_.windowed_width, &user_object_.windowed_height));
    if (fly_) {
        auto& cn = scene_.get_node(user_object_.cameras.camera_node_name());
        user_object_.position = cn.position();
        user_object_.angles = cn.rotation();
    }
    if (rotate_) {
        auto& on = scene_.get_node(user_object_.obj_node_name);
        user_object_.obj_position = on.position();
        user_object_.obj_angles = on.rotation();
    }
}

FlyingCameraLogic::~FlyingCameraLogic() = default;

void FlyingCameraLogic::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    if (button_press_.key_pressed({.key = "ESCAPE", .gamepad_button = "START"})) {
        std::lock_guard lock{user_object_.focuses.mutex};
        Focus focus = user_object_.focuses.focus();
        if (focus == Focus::MENU) {
            if (user_object_.focuses.size() > 1) {
                user_object_.focuses.pop_back();
            }
        } else if (user_object_.focuses.countdown_active() || any(focus & (Focus::LOADING | Focus::SCENE | Focus::GAME_OVER))) {
            user_object_.focuses.push_back(Focus::MENU);
        } else if (user_object_.focuses.game_over_countdown_active()) {
            // Do nothing, menu will show automatically after the countdown is finished
        } else if (focus != Focus::BASE) {
            throw std::runtime_error("Unknown focus value: " + std::to_string((int)focus));
        }
    }
#ifndef __ANDROID__
    if (button_press_.key_pressed({.key = "F11"})) {
        toggle_fullscreen(window_, user_object_.window_position);
    }
#endif
    LOG_FUNCTION("FlyingCameraLogic::render");
    auto& cn = scene_.get_node(user_object_.cameras.camera_node_name());
    if (fly_) {
#ifdef __ANDROID__
        flying_key_callback(button_press_, user_object_);
#else
        flying_key_callback(window_, button_press_, user_object_);
#endif
        cn.set_position(user_object_.position);
        cn.set_rotation(user_object_.angles);
    } else {
        nofly_key_callback(button_press_, user_object_);
    }
    if (rotate_) {
        auto& on = scene_.get_node(user_object_.obj_node_name);
        on.set_position(user_object_.obj_position);
        on.set_rotation(user_object_.obj_angles);
    }
}

void FlyingCameraLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "FlyingCameraLogic\n";
}
