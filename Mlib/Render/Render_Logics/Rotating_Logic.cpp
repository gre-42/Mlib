#include "Rotating_Logic.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/RenderGuards.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/linmath.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>

using namespace Mlib;

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    GLFW_CHK(RotatingLogicUserClass* user_object = (RotatingLogicUserClass*)glfwGetWindowUserPointer(window));
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        GLFW_CHK(glfwSetWindowShouldClose(window, GLFW_TRUE));
    }
    if (action == GLFW_REPEAT || action == GLFW_PRESS) {
        if (mods & GLFW_MOD_SHIFT) {
            if ((user_object->beacon_locations != nullptr) &&
                !user_object->beacon_locations->empty())
            {
                user_object->beacon_index = std::clamp<size_t>(user_object->beacon_index, 0, user_object->beacon_locations->size() - 1);
                switch(key) {
                    case GLFW_KEY_UP:
                        user_object->beacon_index += std::min<size_t>(1, user_object->beacon_locations->size() - 1 - user_object->beacon_index);
                        std::cerr << "Beacon index: " << user_object->beacon_index << std::endl;
                        break;
                    case GLFW_KEY_DOWN:
                        user_object->beacon_index -= std::min<size_t>(1, user_object->beacon_index);
                        std::cerr << "Beacon index: " << user_object->beacon_index << std::endl;
                        break;
                    case GLFW_KEY_PAGE_UP:
                        user_object->beacon_index += std::min<size_t>(10, user_object->beacon_locations->size() - 1 - user_object->beacon_index);
                        std::cerr << "Beacon index: " << user_object->beacon_index << std::endl;
                        break;
                    case GLFW_KEY_PAGE_DOWN:
                        user_object->beacon_index -= std::min<size_t>(10, user_object->beacon_index);
                        std::cerr << "Beacon index: " << user_object->beacon_index << std::endl;
                        break;
                    case GLFW_KEY_HOME:
                        user_object->beacon_index += std::min<size_t>(100, user_object->beacon_locations->size() - 1 - user_object->beacon_index);
                        std::cerr << "Beacon index: " << user_object->beacon_index << std::endl;
                        break;
                    case GLFW_KEY_END:
                        user_object->beacon_index -= std::min<size_t>(100, user_object->beacon_index);
                        std::cerr << "Beacon index: " << user_object->beacon_index << std::endl;
                        break;
                }
            }
        } else {
            switch (key) {
                case GLFW_KEY_UP:
                    user_object->camera_z -= 0.1f;
                    break;
                case GLFW_KEY_DOWN:
                    user_object->camera_z += 0.1f;
                    break;
                case GLFW_KEY_LEFT:
                    user_object->angle_y += 0.04f;
                    break;
                case GLFW_KEY_RIGHT:
                    user_object->angle_y -= 0.04f;
                    break;
                case GLFW_KEY_PAGE_UP:
                    user_object->angle_x += 0.04f;
                    break;
                case GLFW_KEY_PAGE_DOWN:
                    user_object->angle_x -= 0.04f;
                    break;
                case GLFW_KEY_KP_ADD:
                    user_object->scale += 0.04f;
                    break;
                case GLFW_KEY_KP_SUBTRACT:
                    user_object->scale -= 0.04f;
                    break;
            }
        }
    }
    fullscreen_callback(window, key, scancode, action, mods);
}

RotatingLogic::RotatingLogic(
    GLFWwindow* window,
    const Scene& scene,
    bool rotate,
    float scale,
    float camera_z,
    const std::vector<TransformationMatrix<float, 3>>* beacon_locations)
: scene_{scene},
  rotate_{rotate}
{
    user_object_.scale = scale;
    user_object_.camera_z = camera_z;
    user_object_.beacon_locations = beacon_locations;
    GLFW_CHK(glfwGetWindowPos(window, &user_object_.windowed_x, &user_object_.windowed_y));
    GLFW_CHK(glfwGetWindowSize(window, &user_object_.windowed_width, &user_object_.windowed_height));
    GLFW_CHK(glfwSetWindowUserPointer(window, &user_object_));
    GLFW_CHK(glfwSetKeyCallback(window, key_callback));
}

void RotatingLogic::render(
    int width,
    int height,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("RotatingLogic::render");
    RenderToScreenGuard rsg;
    float aspect_ratio = width / (float) height;

    auto cn = scene_.get_node("camera");
    cn->set_position(FixedArray<float, 3>{0.f, 0.f, user_object_.camera_z});
    auto co = cn->get_camera()->copy();
    co->set_aspect_ratio(aspect_ratio);
    FixedArray<float, 4, 4> vp = dot2d(
        co->projection_matrix(),
        cn->absolute_view_matrix().affine());
    TransformationMatrix<float, 3> iv = cn->absolute_model_matrix();

    if (user_object_.scale != 1 || rotate_ || user_object_.angle_x != 0 || user_object_.angle_y != 0) {
        auto on = scene_.get_node("obj");
        on->set_scale(user_object_.scale);
        on->set_rotation(FixedArray<float, 3>{
            user_object_.angle_x,
            rotate_ ? (float)glfwGetTime() : user_object_.angle_y,
            0.f});
    }
    if ((user_object_.beacon_locations != nullptr) && !user_object_.beacon_locations->empty()) {
        SceneNode* bn = scene_.get_node("obj")->get_child("beacon");
        size_t beacon_index = std::clamp<size_t>(user_object_.beacon_index, 0, user_object_.beacon_locations->size() - 1);
        const TransformationMatrix<float, 3> pose = (*user_object_.beacon_locations)[beacon_index];
        float scale = pose.get_scale();
        bn->set_relative_pose(pose.t(), matrix_2_tait_bryan_angles(pose.R() / scale), scale);
    }

    RenderConfigGuard rcg{ render_config };

    // make sure we clear the framebuffer's content
    CHK(glClearColor(
        render_config.background_color(0),
        render_config.background_color(1),
        render_config.background_color(2),
        1));
    CHK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    scene_.render(vp, iv, render_config, scene_graph_config, frame_id.external_render_pass);
}

float RotatingLogic::near_plane() const {
    return 1;
}

float RotatingLogic::far_plane() const {
    return 100;
}

const FixedArray<float, 4, 4>& RotatingLogic::vp() const {
    throw std::runtime_error("RotatingLogic::vp not implemented");
}

const TransformationMatrix<float, 3>& RotatingLogic::iv() const {
    throw std::runtime_error("RotatingLogic::iv not implemented");
}

bool RotatingLogic::requires_postprocessing() const {
    return true;
}

void RotatingLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "RotatingLogic\n";
}
