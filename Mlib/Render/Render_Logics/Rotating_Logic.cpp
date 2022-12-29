#ifndef __ANDROID__

#include "Rotating_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/Render_Guards.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Render/linmath.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

static void key_callback(
    GLFWwindow& window,
    ButtonPress& button_press,
    RotatingLogicUserClass& user_object)
{
    if (button_press.key_pressed({.key = "ESCAPE"})) {
        GLFW_CHK(glfwSetWindowShouldClose(&window, GLFW_TRUE));
    }
    if (button_press.key_down({.key = "SHIFT"})) {
        if ((user_object.beacon_locations != nullptr) &&
            !user_object.beacon_locations->empty())
        {
            user_object.beacon_index = std::clamp<size_t>(user_object.beacon_index, 0, user_object.beacon_locations->size() - 1);
            if (button_press.key_down({.key = "UP"})) {
                user_object.beacon_index += std::min<size_t>(1, user_object.beacon_locations->size() - 1 - user_object.beacon_index);
                std::cerr << "Beacon index: " << user_object.beacon_index << std::endl;
            }
            if (button_press.key_down({.key = "DOWN"})) {
                user_object.beacon_index -= std::min<size_t>(1, user_object.beacon_index);
                std::cerr << "Beacon index: " << user_object.beacon_index << std::endl;
            }
            if (button_press.key_down({.key = "PAGE_UP"})) {
                user_object.beacon_index += std::min<size_t>(10, user_object.beacon_locations->size() - 1 - user_object.beacon_index);
                std::cerr << "Beacon index: " << user_object.beacon_index << std::endl;
            }
            if (button_press.key_down({.key = "PAGE_DOWN"})) {
                user_object.beacon_index -= std::min<size_t>(10, user_object.beacon_index);
                std::cerr << "Beacon index: " << user_object.beacon_index << std::endl;
            }
            if (button_press.key_down({.key = "HOME"})) {
                user_object.beacon_index += std::min<size_t>(100, user_object.beacon_locations->size() - 1 - user_object.beacon_index);
                std::cerr << "Beacon index: " << user_object.beacon_index << std::endl;
            }
            if (button_press.key_down({.key = "END"})) {
                user_object.beacon_index -= std::min<size_t>(100, user_object.beacon_index);
                std::cerr << "Beacon index: " << user_object.beacon_index << std::endl;
            }
        }
    } else {
        if (button_press.key_down({.key = "UP"})) {
            user_object.camera_z -= 0.1f;
        }
        if (button_press.key_down({.key = "DOWN"})) {
            user_object.camera_z += 0.1f;
        }
        if (button_press.key_down({.key = "LEFT"})) {
            user_object.angle_y += 0.04f;
        }
        if (button_press.key_down({.key = "RIGHT"})) {
            user_object.angle_y -= 0.04f;
        }
        if (button_press.key_down({.key = "PAGE_UP"})) {
            user_object.angle_x += 0.04f;
        }
        if (button_press.key_down({.key = "PAGE_DOWN"})) {
            user_object.angle_x -= 0.04f;
        }
        if (button_press.key_down({.key = "KP_ADD"})) {
            user_object.scale += 0.04f;
        }
        if (button_press.key_down({.key = "KP_SUBTRACT"})) {
            user_object.scale -= 0.04f;
        }
    }
}

RotatingLogic::RotatingLogic(
    ButtonStates& button_states,
    GLFWwindow& window,
    const Scene& scene,
    bool rotate,
    float scale,
    float camera_z,
    const FixedArray<float, 3>& background_color,
    const std::vector<TransformationMatrix<float, double, 3>>* beacon_locations)
: button_press_{button_states},
  window_{window},
  scene_{scene},
  rotate_{rotate},
  background_color_{background_color}
{
    user_object_.scale = scale;
    user_object_.camera_z = camera_z;
    user_object_.beacon_locations = beacon_locations;
    GLFW_CHK(glfwGetWindowPos(&window, &user_object_.windowed_x, &user_object_.windowed_y));
    GLFW_CHK(glfwGetWindowSize(&window, &user_object_.windowed_width, &user_object_.windowed_height));
    GLFW_CHK(glfwSetWindowUserPointer(&window, &user_object_));
}

void RotatingLogic::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    key_callback(window_, button_press_, user_object_);

    std::lock_guard lock{ scene_.delete_node_mutex() };

    LOG_FUNCTION("RotatingLogic::render");
    RenderToScreenGuard rsg;
    float aspect_ratio = width / (float) height;

    auto& cn = scene_.get_node("camera");
    cn.set_position(FixedArray<double, 3>{0.f, 0.f, user_object_.camera_z});
    auto co = cn.get_camera().copy();
    co->set_aspect_ratio(aspect_ratio);
    FixedArray<double, 4, 4> vp = dot2d(
        co->projection_matrix().casted<double>(),
        cn.absolute_view_matrix().affine());
    TransformationMatrix<float, double, 3> iv = cn.absolute_model_matrix();

    if (user_object_.scale != 1 || rotate_ || user_object_.angle_x != 0 || user_object_.angle_y != 0) {
        auto& on = scene_.get_node("obj");
        on.set_scale(user_object_.scale);
        on.set_rotation(FixedArray<float, 3>{
            user_object_.angle_x,
            rotate_ ? (float)glfwGetTime() : user_object_.angle_y,
            0.f});
    }
    if ((user_object_.beacon_locations != nullptr) && !user_object_.beacon_locations->empty()) {
        auto& bn = scene_.get_node("obj").get_child("beacon");
        size_t beacon_index = std::clamp<size_t>(user_object_.beacon_index, 0, user_object_.beacon_locations->size() - 1);
        const TransformationMatrix<float, double, 3> pose = (*user_object_.beacon_locations)[beacon_index];
        float scale = pose.get_scale();
        bn.set_relative_pose(pose.t(), matrix_2_tait_bryan_angles(pose.R() / scale), scale);
    }

    RenderConfigGuard rcg{ render_config, frame_id.external_render_pass.pass };

    // make sure we clear the framebuffer's content
    CHK(glClearColor(
        background_color_(0),
        background_color_(1),
        background_color_(2),
        1));
    CHK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    scene_.render(vp, iv, cn, render_config, scene_graph_config, frame_id.external_render_pass);
}

float RotatingLogic::near_plane() const {
    return 1;
}

float RotatingLogic::far_plane() const {
    return 100;
}

const FixedArray<double, 4, 4>& RotatingLogic::vp() const {
    THROW_OR_ABORT("RotatingLogic::vp not implemented");
}

const TransformationMatrix<float, double, 3>& RotatingLogic::iv() const {
    THROW_OR_ABORT("RotatingLogic::iv not implemented");
}

bool RotatingLogic::requires_postprocessing() const {
    return true;
}

void RotatingLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "RotatingLogic\n";
}

#endif
