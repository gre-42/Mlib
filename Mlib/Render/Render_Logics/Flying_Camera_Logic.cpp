#include "Flying_Camera_Logic.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Selected_Cameras.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>
#include <Mlib/Set_Fps.hpp>

using namespace Mlib;

static void flying_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    FlyingCameraUserClass* user_object = (FlyingCameraUserClass*)glfwGetWindowUserPointer(window);
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    if (action == GLFW_REPEAT || action == GLFW_PRESS) {
        if (mods & GLFW_MOD_CONTROL) {
            switch(key) {
                case GLFW_KEY_LEFT:
                    user_object->obj_angle_y += 0.01;
                    break;
                case GLFW_KEY_RIGHT:
                    user_object->obj_angle_y -= 0.01;
                    break;
                case GLFW_KEY_PAGE_UP:
                    user_object->obj_angle_x += 0.01;
                    break;
                case GLFW_KEY_PAGE_DOWN:
                    user_object->obj_angle_x -= 0.01;
                    break;
                case GLFW_KEY_KP_ADD:
                    user_object->obj_position(1) += 0.04;
                    break;
                case GLFW_KEY_KP_SUBTRACT:
                    user_object->obj_position(1) -= 0.04;
                    break;
            }
        } else {
            switch(key) {
                case GLFW_KEY_UP:
                    user_object->position_z -= 0.04;
                    break;
                case GLFW_KEY_DOWN:
                    user_object->position_z += 0.04;
                    break;
                case GLFW_KEY_LEFT:
                    user_object->angle_y += 0.01;
                    break;
                case GLFW_KEY_RIGHT:
                    user_object->angle_y -= 0.01;
                    break;
                case GLFW_KEY_PAGE_UP:
                    user_object->angle_x += 0.01;
                    break;
                case GLFW_KEY_PAGE_DOWN:
                    user_object->angle_x -= 0.01;
                    break;
                case GLFW_KEY_KP_ADD:
                    user_object->position_y += 0.04;
                    break;
                case GLFW_KEY_KP_SUBTRACT:
                    user_object->position_y -= 0.04;
                    break;
            }
        }
    }
    fullscreen_callback(window, key, scancode, action, mods);
}

static void nofly_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    FlyingCameraUserClass* user_object = (FlyingCameraUserClass*)glfwGetWindowUserPointer(window);
    // if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    //     glfwSetWindowShouldClose(window, GLFW_TRUE);
    // }
    if (action == GLFW_REPEAT || action == GLFW_PRESS) {
        switch(key) {
            case GLFW_KEY_L:
            {
                auto& cams = user_object->cameras.camera_cycle_far;
                if (cams.empty()) {
                    throw std::runtime_error("Far camera cycle is empty");
                }
                auto it = std::find(cams.begin(), cams.end(), user_object->cameras.camera_node_name);
                if (it == cams.end() || ++it == cams.end()) {
                    it = cams.begin();
                }
                user_object->cameras.camera_node_name = *it;
                break;
            }
            case GLFW_KEY_P:
                if (user_object->physics_set_fps != nullptr) {
                    user_object->physics_set_fps->toggle_pause_resume();
                }
                break;
        }
    }
    fullscreen_callback(window, key, scancode, action, mods);
}

FlyingCameraLogic::FlyingCameraLogic(
    const Scene& scene,
    FlyingCameraUserClass& user_object,
    bool fly,
    bool rotate)
: scene_{scene},
  user_object_{user_object},
  window_{nullptr},
  fly_{fly},
  rotate_{rotate}
{}

void FlyingCameraLogic::initialize(GLFWwindow* window) {
    if (window_ != nullptr) {
        throw std::runtime_error("Multiple calls to FlyingCameraLogic::initialize");
    }
    window_ = window;
    // glfwGetWindowPos(window, &user_object_.window_x, &user_object_.window_y);
    // glfwGetWindowSize(window, &user_object_.window_width, &user_object_.window_height);
    glfwSetWindowUserPointer(window, &user_object_);
    if (fly_ || rotate_) {
        glfwSetKeyCallback(window, flying_key_callback);

        if (fly_) {
            auto cn = scene_.get_node(user_object_.cameras.camera_node_name);
            user_object_.position_y = cn->position()(1);
            user_object_.position_z = cn->position()(2);
            user_object_.angle_x = cn->rotation()(0);
            user_object_.angle_y = cn->rotation()(1);
        }
        if (rotate_) {
            auto on = scene_.get_node("obj");
            user_object_.obj_angle_x = on->rotation()(0);
            user_object_.obj_angle_y = on->rotation()(1);
            user_object_.obj_position = on->position();
        }
    } else {
        glfwSetKeyCallback(window, nofly_key_callback);
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
    button_press_.update(window_);
    if (button_press_.key_pressed({key: "ESCAPE", gamepad_button: "START"})) {
        if (user_object_.focus == Focus::MENU) {
            user_object_.focus = Focus::SCENE;
        } else if ((user_object_.focus == Focus::SCENE) || (user_object_.focus == Focus::COUNTDOWN)) {
            user_object_.focus = Focus::MENU;
        } else {
            throw std::runtime_error("Unknown focus value");
        }
    }

    LOG_FUNCTION("FlyingCameraLogic::render");
    SceneNode* cn = scene_.get_node(user_object_.cameras.camera_node_name);
    if (fly_) {
        cn->set_position({0, user_object_.position_y, user_object_.position_z});
        cn->set_rotation({user_object_.angle_x, user_object_.angle_y, 0});
    }
    if (rotate_) {
        SceneNode* on = scene_.get_node(user_object_.obj_node_name);
        on->set_rotation({user_object_.obj_angle_x, user_object_.obj_angle_y, 0});
        on->set_position(user_object_.obj_position);
    }
}

float FlyingCameraLogic::near_plane() const {
    throw std::runtime_error("FlyingCameraLogic::near_plane not implemented");
}

float FlyingCameraLogic::far_plane() const {
    throw std::runtime_error("FlyingCameraLogic::far_plane not implemented");
}

const FixedArray<float, 4, 4>& FlyingCameraLogic::vp() const {
    throw std::runtime_error("FlyingCameraLogic::vp not implemented");
}

bool FlyingCameraLogic::requires_postprocessing() const {
    throw std::runtime_error("FlyingCameraLogic::requires_postprocessing not implemented");
}
