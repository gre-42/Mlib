#include "Flying_Camera_Logic.hpp"
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Combination.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Render/Selected_Cameras/Camera_Cycle_Type.hpp>
#include <Mlib/Render/Selected_Cameras/Selected_Cameras.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Render/Ui/Cursor_States.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>
#include <Mlib/Threads/Termination_Manager.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <Mlib/Time/Fps/Set_Fps.hpp>

namespace Mlib {
struct FlyingCameraLogicKeys {
    BaseKeyCombination V{{{.key = "V"}}};
    BaseKeyCombination W{{{.key = "W"}}};
    BaseKeyCombination D{{{.key = "D"}}};
    BaseKeyCombination C{{{.key = "C"}}};
};
}

using namespace Mlib;

static void flying_key_callback(
#ifndef __ANDROID__
    ButtonPress& button_press,
    FlyingCameraUserClass& user_object,
    FlyingCameraLogicKeys& keys)
{
#else
    ButtonPress& button_press,
    FlyingCameraUserClass& user_object)
{
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
            user_object.position -= (0.2f * tait_bryan_angles_2_matrix(user_object.angles).column(2)).casted<double>();
            // user_object.position(2) -= 0.04f;
        }
        if (button_press.key_down({.key = "DOWN"})) {
            user_object.position += (0.2f * tait_bryan_angles_2_matrix(user_object.angles).column(2)).casted<double>();
            // user_object.position(2) += 0.04f;
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
            user_object.position(1) += 0.2f;
        }
        if (button_press.key_down({.key = "KP_SUBTRACT"})) {
            user_object.position(1) -= 0.2f;
        }
    }
}

static void nofly_key_callback(
    ButtonPress& button_press,
    FlyingCameraUserClass& user_object,
    FlyingCameraLogicKeys& keys)
{
    // if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    //     GLFW_CHK(glfwSetWindowShouldClose(window, GLFW_TRUE));
    // }
    if (button_press.keys_pressed(keys.V)) {
        user_object.cameras.cycle_camera(CameraCycleType::FAR);
    }
    // if (button_press.key_pressed({.key = "P"})) {
    //     if (user_object.physics_set_fps != nullptr) {
    //         user_object.physics_set_fps->toggle_pause_resume();
    //     }
    // }
    if (button_press.key_down({.key = "LEFT_CONTROL"})) {
        if (button_press.keys_pressed(keys.W)) {
            user_object.wire_frame = zapped(user_object.wire_frame);
        }
        if (button_press.keys_pressed(keys.D)) {
            user_object.depth_test = zapped(user_object.depth_test);
        }
        if (button_press.keys_pressed(keys.C)) {
            user_object.cull_faces = zapped(user_object.cull_faces);
        }
    }
}

FlyingCameraLogic::FlyingCameraLogic(
        const Scene &scene,
        FlyingCameraUserClass &user_object,
        bool fly,
        bool rotate)
    : scene_{scene}
    , user_object_{user_object}
    , button_press_{user_object.button_states}
    , fly_{fly}
    , rotate_{rotate}
    , keys_{std::make_unique<FlyingCameraLogicKeys>()}
{
    // GLFW_CHK(glfwGetWindowPos(window, &user_object_.windowed_x, &user_object_.windowed_y));
    // GLFW_CHK(glfwGetWindowSize(window, &user_object_.windowed_width, &user_object_.windowed_height));
    if (fly_) {
        DanglingRef<SceneNode> cn = scene_.get_node(user_object_.cameras.camera_node_name(), DP_LOC);
        user_object_.position = cn->position();
        user_object_.angles = cn->rotation();
    }
    if (rotate_) {
        DanglingRef<SceneNode> on = scene_.get_node(user_object_.obj_node_name, DP_LOC);
        user_object_.obj_position = on->position();
        user_object_.obj_angles = on->rotation();
    }
}

FlyingCameraLogic::~FlyingCameraLogic() = default;

void FlyingCameraLogic::render(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    LOG_FUNCTION("FlyingCameraLogic::render");
    DanglingRef<SceneNode> cn = scene_.get_node(user_object_.cameras.camera_node_name(), DP_LOC);
    if (fly_) {
#ifdef __ANDROID__
        flying_key_callback(button_press_, user_object_);
#else
        flying_key_callback(button_press_, user_object_, *keys_);
#endif
        cn->set_position(user_object_.position, SUCCESSOR_POSE);
        cn->set_rotation(user_object_.angles, SUCCESSOR_POSE);
    } else {
        nofly_key_callback(button_press_, user_object_, *keys_);
    }
    if (rotate_) {
        DanglingRef<SceneNode> on = scene_.get_node(user_object_.obj_node_name, DP_LOC);
        on->set_position(user_object_.obj_position, SUCCESSOR_POSE);
        on->set_rotation(user_object_.obj_angles, SUCCESSOR_POSE);
    }
}

void FlyingCameraLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "FlyingCameraLogic\n";
}
