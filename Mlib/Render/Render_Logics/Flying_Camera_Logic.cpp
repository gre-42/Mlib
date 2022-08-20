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

static void flying_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    GLFW_CHK(FlyingCameraUserClass* user_object = (FlyingCameraUserClass*)glfwGetWindowUserPointer(window));
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        GLFW_CHK(glfwSetWindowShouldClose(window, GLFW_TRUE));
    }
    if (action == GLFW_REPEAT || action == GLFW_PRESS) {
        if (mods & GLFW_MOD_CONTROL) {
            switch(key) {
                case GLFW_KEY_LEFT:
                    user_object->obj_angles(1) += 0.01f;
                    break;
                case GLFW_KEY_RIGHT:
                    user_object->obj_angles(1) -= 0.01f;
                    break;
                case GLFW_KEY_PAGE_UP:
                    user_object->obj_angles(0) += 0.01f;
                    break;
                case GLFW_KEY_PAGE_DOWN:
                    user_object->obj_angles(0) -= 0.01f;
                    break;
                case GLFW_KEY_KP_ADD:
                    user_object->obj_position(1) += 0.04f;
                    break;
                case GLFW_KEY_KP_SUBTRACT:
                    user_object->obj_position(1) -= 0.04f;
                    break;
            }
        } else {
            switch(key) {
                case GLFW_KEY_UP:
                    user_object->position(2) -= 0.04f;
                    break;
                case GLFW_KEY_DOWN:
                    user_object->position(2) += 0.04f;
                    break;
                case GLFW_KEY_LEFT:
                    user_object->angles(1) += 0.01f;
                    break;
                case GLFW_KEY_RIGHT:
                    user_object->angles(1) -= 0.01f;
                    break;
                case GLFW_KEY_PAGE_UP:
                    user_object->angles(0) += 0.01f;
                    break;
                case GLFW_KEY_PAGE_DOWN:
                    user_object->angles(0) -= 0.01f;
                    break;
                case GLFW_KEY_KP_ADD:
                    user_object->position(1) += 0.04f;
                    break;
                case GLFW_KEY_KP_SUBTRACT:
                    user_object->position(1) -= 0.04f;
                    break;
            }
        }
    }
    fullscreen_callback(window, key, scancode, action, mods);
    if (!unhandled_exceptions_occured()) {
        try {
            user_object->button_states.notify_key_event(key, action);
        } catch (const std::runtime_error&) {
            add_unhandled_exception(std::current_exception());
        }
    }
}

static void nofly_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    GLFW_CHK(FlyingCameraUserClass* user_object = (FlyingCameraUserClass*)glfwGetWindowUserPointer(window));
    // if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    //     GLFW_CHK(glfwSetWindowShouldClose(window, GLFW_TRUE));
    // }
    try {
        if (action == GLFW_REPEAT || action == GLFW_PRESS) {
            switch(key) {
                case GLFW_KEY_L:
                {
                    auto& cams = user_object->cameras.camera_cycle_far;
                    if (cams.empty()) {
                        throw std::runtime_error("Far camera cycle is empty");
                    }
                    std::string old_camera_node_name;
                    {
                        std::lock_guard lock{ user_object->delete_node_mutex };
                        old_camera_node_name = user_object->cameras.camera_node_name();
                    }
                    auto it = std::find(cams.begin(), cams.end(), old_camera_node_name);
                    if (it == cams.end() || ++it == cams.end()) {
                        it = cams.begin();
                    }
                    user_object->cameras.set_camera_node_name(*it);
                    break;
                }
                // case GLFW_KEY_P:
                //     if (user_object->physics_set_fps != nullptr) {
                //         user_object->physics_set_fps->toggle_pause_resume();
                //     }
                //     break;
                case GLFW_KEY_W:
                    if (mods & GLFW_MOD_CONTROL) {
                        user_object->wire_frame = zapped(user_object->wire_frame);
                    }
                    break;
                case GLFW_KEY_D:
                    if (mods & GLFW_MOD_CONTROL) {
                        user_object->depth_test = zapped(user_object->depth_test);
                    }
                    break;
                case GLFW_KEY_C:
                    if (mods & GLFW_MOD_CONTROL) {
                        user_object->cull_faces = zapped(user_object->cull_faces);
                    }
                    break;
            }
        }
    } catch (const std::runtime_error&) {
        add_unhandled_exception(std::current_exception());
    }
    if (!unhandled_exceptions_occured()) {
        fullscreen_callback(window, key, scancode, action, mods);
        try {
            user_object->button_states.notify_key_event(key, action);
        } catch (const std::exception&) {
            add_unhandled_exception(std::current_exception());
        }
    }
}

static void nofly_cursor_callback(GLFWwindow* window, double xpos, double ypos)
{
    GLFW_CHK(FlyingCameraUserClass* user_object = (FlyingCameraUserClass*)glfwGetWindowUserPointer(window));
    try {
        user_object->cursor_states.update_cursor(xpos, ypos);
        GLFW_CHK(glfwSetCursorPos(window, 0, 0));
    } catch (const std::runtime_error&) {
        add_unhandled_exception(std::current_exception());
    }
}

static void nofly_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    try {
        GLFW_CHK(FlyingCameraUserClass* user_object = (FlyingCameraUserClass*)glfwGetWindowUserPointer(window));
        user_object->button_states.notify_mouse_button_event(button, action);
    } catch (const std::runtime_error&) {
        add_unhandled_exception(std::current_exception());
    }
}

static void nofly_scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    try {
        GLFW_CHK(FlyingCameraUserClass* user_object = (FlyingCameraUserClass*)glfwGetWindowUserPointer(window));
        user_object->scroll_wheel_states.update_cursor(xoffset, yoffset);
    } catch (const std::runtime_error&) {
        add_unhandled_exception(std::current_exception());
    }
}

FlyingCameraLogic::FlyingCameraLogic(
    GLFWwindow* window,
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
{
    // GLFW_CHK(glfwGetWindowPos(window, &user_object_.windowed_x, &user_object_.windowed_y));
    // GLFW_CHK(glfwGetWindowSize(window, &user_object_.windowed_width, &user_object_.windowed_height));
    GLFW_CHK(glfwSetWindowUserPointer(window, &user_object_));
    if (fly_ || rotate_) {
        GLFW_CHK(glfwSetKeyCallback(window, flying_key_callback));

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
    } else {
        GLFW_CHK(glfwSetKeyCallback(window, nofly_key_callback));
        GLFW_CHK(glfwSetCursorPosCallback(window, nofly_cursor_callback));
        GLFW_CHK(glfwSetScrollCallback(window, nofly_scroll_callback));
        GLFW_CHK(glfwSetMouseButtonCallback(window, nofly_mouse_button_callback));
    }
}

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
        } else if (focus != Focus::BASE) {
            throw std::runtime_error("Unknown focus value");
        }
    }

    LOG_FUNCTION("FlyingCameraLogic::render");
    auto& cn = scene_.get_node(user_object_.cameras.camera_node_name());
    if (fly_) {
        cn.set_position(user_object_.position);
        cn.set_rotation(user_object_.angles);
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
