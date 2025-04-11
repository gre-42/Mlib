#ifndef __ANDROID__

#include "Rotating_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Layout/Layout_Constraint_Parameters.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation/Bijection.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/Render_Guards.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Combination.hpp>
#include <Mlib/Render/Key_Bindings/Key_Configuration.hpp>
#include <Mlib/Render/Key_Bindings/Lockable_Key_Configurations.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Render/linmath.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

namespace Mlib {
class RotatingLogicKeys {
public:
    explicit RotatingLogicKeys(ButtonStates& button_states)
        : escape{ button_states, key_configurations, "escape", "" }
    {
        key_configurations
            .lock_exclusive_for(std::chrono::seconds(2), "Key configurations")
            ->emplace()
            .insert("escape", { {{{.key = "ESCAPE"}}} });
    }
    ButtonPress escape;
private:
    LockableKeyConfigurations key_configurations;
};
}

using namespace Mlib;

static void key_callback(
    GLFWwindow& window,
    ButtonStates& button_states,
    RotatingLogicUserClass& user_object,
    RotatingLogicKeys& keys)
{
    if (keys.escape.keys_pressed()) {
        GLFW_CHK(glfwSetWindowShouldClose(&window, GLFW_TRUE));
    }
    if (button_states.key_down({.key = "LEFT_SHIFT"})) {
        if ((user_object.beacon_locations != nullptr) &&
            !user_object.beacon_locations->empty())
        {
            user_object.beacon_index = std::clamp<size_t>(user_object.beacon_index, 0, user_object.beacon_locations->size() - 1);
            if (button_states.key_down({.key = "UP"})) {
                user_object.beacon_index += std::min<size_t>(1, user_object.beacon_locations->size() - 1 - user_object.beacon_index);
                lerr() << "Beacon index: " << user_object.beacon_index;
            }
            if (button_states.key_down({.key = "DOWN"})) {
                user_object.beacon_index -= std::min<size_t>(1, user_object.beacon_index);
                lerr() << "Beacon index: " << user_object.beacon_index;
            }
            if (button_states.key_down({.key = "PAGE_UP"})) {
                user_object.beacon_index += std::min<size_t>(10, user_object.beacon_locations->size() - 1 - user_object.beacon_index);
                lerr() << "Beacon index: " << user_object.beacon_index;
            }
            if (button_states.key_down({.key = "PAGE_DOWN"})) {
                user_object.beacon_index -= std::min<size_t>(10, user_object.beacon_index);
                lerr() << "Beacon index: " << user_object.beacon_index;
            }
            if (button_states.key_down({.key = "HOME"})) {
                user_object.beacon_index += std::min<size_t>(100, user_object.beacon_locations->size() - 1 - user_object.beacon_index);
                lerr() << "Beacon index: " << user_object.beacon_index;
            }
            if (button_states.key_down({.key = "END"})) {
                user_object.beacon_index -= std::min<size_t>(100, user_object.beacon_index);
                lerr() << "Beacon index: " << user_object.beacon_index;
            }
        }
    } else {
        if (button_states.key_down({.key = "UP"})) {
            user_object.camera_z -= 0.1f;
        }
        if (button_states.key_down({.key = "DOWN"})) {
            user_object.camera_z += 0.1f;
        }
        if (button_states.key_down({.key = "LEFT"})) {
            user_object.angle_y += 0.004f;
        }
        if (button_states.key_down({.key = "RIGHT"})) {
            user_object.angle_y -= 0.004f;
        }
        if (button_states.key_down({.key = "PAGE_UP"})) {
            user_object.angle_x += 0.004f;
        }
        if (button_states.key_down({.key = "PAGE_DOWN"})) {
            user_object.angle_x -= 0.004f;
        }
        if (button_states.key_down({.key = "KP_ADD"})) {
            user_object.scale += 0.04f;
        }
        if (button_states.key_down({.key = "KP_SUBTRACT"})) {
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
    const std::vector<TransformationMatrix<float, ScenePos, 3>>* beacon_locations)
    : window_{window}
    , scene_{scene}
    , button_states_{ button_states }
    , rotate_{rotate}
    , background_color_{background_color}
    , keys_{ std::make_unique<RotatingLogicKeys>(button_states) }
{
    user_object_.scale = scale;
    user_object_.camera_z = camera_z;
    user_object_.beacon_locations = beacon_locations;
    GLFW_CHK(glfwGetWindowPos(&window, &user_object_.windowed_x, &user_object_.windowed_y));
    GLFW_CHK(glfwGetWindowSize(&window, &user_object_.windowed_width, &user_object_.windowed_height));
    GLFW_CHK(glfwSetWindowUserPointer(&window, &user_object_));
}

RotatingLogic::~RotatingLogic() = default;

std::optional<RenderSetup> RotatingLogic::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return std::nullopt;
}

void RotatingLogic::render_with_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id,
    const RenderSetup& setup)
{
    LOG_FUNCTION("RotatingLogic::render");

    key_callback(window_, button_states_, user_object_, *keys_);

    notify_rendering(CURRENT_SOURCE_LOCATION);
    float aspect_ratio = lx.flength() / ly.flength();

    DanglingRef<SceneNode> cn = scene_.get_node("camera", DP_LOC);
    cn->set_position(
        FixedArray<ScenePos, 3>{0.f, 0.f, user_object_.camera_z},
        std::nullopt);
    auto co = cn->get_camera(CURRENT_SOURCE_LOCATION)->copy();
    co->set_aspect_ratio(aspect_ratio);
    auto bi = cn->absolute_bijection(frame_id.external_render_pass.time);
    FixedArray<ScenePos, 4, 4> vp = dot2d(
        co->projection_matrix().casted<ScenePos>(),
        bi.view.affine());

    if (user_object_.scale != 1 || rotate_ || user_object_.angle_x != 0 || user_object_.angle_y != 0) {
        DanglingRef<SceneNode> on = scene_.get_node("obj", DP_LOC);
        on->set_scale(user_object_.scale);
        on->set_rotation(FixedArray<float, 3>{
            user_object_.angle_x,
            rotate_ ? (float)glfwGetTime() : user_object_.angle_y,
            0.f},
            std::nullopt);
    }
    if ((user_object_.beacon_locations != nullptr) && !user_object_.beacon_locations->empty()) {
        DanglingRef<SceneNode> bn = scene_.get_node("obj", DP_LOC)->get_child("beacon");
        size_t beacon_index = std::clamp<size_t>(user_object_.beacon_index, 0, user_object_.beacon_locations->size() - 1);
        const TransformationMatrix<float, ScenePos, 3> pose = (*user_object_.beacon_locations)[beacon_index];
        float scale = pose.get_scale();
        bn->set_relative_pose(pose.t, matrix_2_tait_bryan_angles(pose.R / scale), scale, std::nullopt);
    }

    RenderConfigGuard rcg{ render_config, frame_id.external_render_pass.pass };

    // make sure we clear the framebuffer's content
    CHK(glClearColor(
        background_color_(0),
        background_color_(1),
        background_color_(2),
        1));
    CHK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    DanglingPtr<const SceneNode> cn_ptr = cn.ptr();
    scene_.render(vp, bi.model, cn_ptr, render_config, scene_graph_config, frame_id.external_render_pass);
}

void RotatingLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "RotatingLogic\n";
}

#endif
