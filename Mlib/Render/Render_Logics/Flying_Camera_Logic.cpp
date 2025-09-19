#include "Flying_Camera_Logic.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Combination.hpp>
#include <Mlib/Render/Key_Bindings/Key_Configuration.hpp>
#include <Mlib/Render/Key_Bindings/Lockable_Key_Configurations.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Render/Selected_Cameras/Camera_Cycle_Type.hpp>
#include <Mlib/Render/Selected_Cameras/Selected_Cameras.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Render/Ui/Cursor_States.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Rendered_Scene_Descriptor.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>
#include <Mlib/Threads/Termination_Manager.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <Mlib/Time/Fps/Set_Fps.hpp>

namespace Mlib {
class FlyingCameraLogicKeys {
public:
    explicit FlyingCameraLogicKeys(ButtonStates& button_states)
        : v{ button_states, key_configurations, 0, "v", "" }
        , w{ button_states, key_configurations, 0, "w", "" }
        , d{ button_states, key_configurations, 0, "d", "" }
        , c{ button_states, key_configurations, 0, "c", "" }
    {
        auto lock = key_configurations.lock_exclusive_for(std::chrono::seconds(2), "Key configurations");
        lock->insert(0, "v", { {{{.key = "V"}}} });
        lock->insert(0, "w", { {{{.key = "W"}}} });
        lock->insert(0, "d", { {{{.key = "D"}}} });
        lock->insert(0, "c", { {{{.key = "C"}}} });
    }
    ButtonPress v;
    ButtonPress w;
    ButtonPress d;
    ButtonPress c;
private:
    LockableKeyConfigurations key_configurations;
};
}

using namespace Mlib;

static void flying_key_callback(
    FlyingCameraUserClass& user_object,
    FlyingCameraLogicKeys& keys)
{
    if (user_object.button_states.key_down({.key = "LEFT_CONTROL"})) {
        if (user_object.button_states.key_down({.key = "UP"})) {
            user_object.obj_angles(2) += 0.01f;
        }
        if (user_object.button_states.key_down({.key = "DOWN"})) {
            user_object.obj_angles(2) -= 0.01f;
        }
        if (user_object.button_states.key_down({.key = "LEFT"})) {
            user_object.obj_angles(1) += 0.01f;
        }
        if (user_object.button_states.key_down({.key = "RIGHT"})) {
            user_object.obj_angles(1) -= 0.01f;
        }
        if (user_object.button_states.key_down({.key = "PAGE_UP"})) {
            user_object.obj_angles(0) += 0.01f;
        }
        if (user_object.button_states.key_down({.key = "PAGE_DOWN"})) {
            user_object.obj_angles(0) -= 0.01f;
        }
        if (user_object.button_states.key_down({.key = "KP_ADD"})) {
            user_object.obj_position(1) += 0.04f;
        }
        if (user_object.button_states.key_down({.key = "KP_SUBTRACT"})) {
            user_object.obj_position(1) -= 0.04f;
        }
    } else {
        if (user_object.button_states.key_down({.key = "UP"})) {
            user_object.position -= (0.2f * tait_bryan_angles_2_matrix(user_object.angles).column(2)).casted<ScenePos>();
            // user_object.position(2) -= 0.04f;
        }
        if (user_object.button_states.key_down({.key = "DOWN"})) {
            user_object.position += (0.2f * tait_bryan_angles_2_matrix(user_object.angles).column(2)).casted<ScenePos>();
            // user_object.position(2) += 0.04f;
        }
        if (user_object.button_states.key_down({.key = "LEFT"})) {
            user_object.angles(1) += 0.01f;
        }
        if (user_object.button_states.key_down({.key = "RIGHT"})) {
            user_object.angles(1) -= 0.01f;
        }
        if (user_object.button_states.key_down({.key = "PAGE_UP"})) {
            user_object.angles(0) += 0.01f;
        }
        if (user_object.button_states.key_down({.key = "PAGE_DOWN"})) {
            user_object.angles(0) -= 0.01f;
        }
        if (user_object.button_states.key_down({.key = "KP_ADD"})) {
            user_object.position(1) += 0.2f;
        }
        if (user_object.button_states.key_down({.key = "KP_SUBTRACT"})) {
            user_object.position(1) -= 0.2f;
        }
    }
}

static void nofly_key_callback(
    FlyingCameraUserClass& user_object,
    FlyingCameraLogicKeys& keys)
{
    // if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    //     GLFW_CHK(glfwSetWindowShouldClose(window, GLFW_TRUE));
    // }
    if (keys.v.keys_pressed()) {
        user_object.cameras.cycle_camera(CameraCycleType::FAR);
    }
    // if (button_press.key_pressed({.key = "P"})) {
    //     if (user_object.physics_set_fps != nullptr) {
    //         user_object.physics_set_fps->toggle_pause_resume();
    //     }
    // }
    if (user_object.button_states.key_down({.key = "LEFT_CONTROL"})) {
        if (keys.w.keys_pressed()) {
            user_object.wire_frame = zapped(user_object.wire_frame);
        }
        if (keys.d.keys_pressed()) {
            user_object.depth_test = zapped(user_object.depth_test);
        }
        if (keys.c.keys_pressed()) {
            user_object.cull_faces = zapped(user_object.cull_faces);
        }
    }
}

FlyingCameraLogic::FlyingCameraLogic(
        const Scene &scene,
        FlyingCameraUserClass &user_object,
        bool fly,
        bool rotate)
    : scene_{ scene }
    , user_object_{ user_object }
    , fly_{ fly }
    , rotate_{ rotate }
    , keys_{ std::make_unique<FlyingCameraLogicKeys>(user_object.button_states) }
{
    // GLFW_CHK(glfwGetWindowPos(window, &user_object_.windowed_x, &user_object_.windowed_y));
    // GLFW_CHK(glfwGetWindowSize(window, &user_object_.windowed_width, &user_object_.windowed_height));
    if (fly_) {
        DanglingBaseClassRef<SceneNode> cn = user_object_.cameras.camera(DP_LOC).node;
        user_object_.position = cn->position();
        user_object_.angles = cn->rotation();
    }
    if (rotate_) {
        DanglingBaseClassRef<SceneNode> on = scene_.get_node(user_object_.obj_node_name, DP_LOC);
        user_object_.obj_position = on->position();
        user_object_.obj_angles = on->rotation();
    }
}

FlyingCameraLogic::~FlyingCameraLogic() {
    on_destroy.clear();
}

std::optional<RenderSetup> FlyingCameraLogic::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    LOG_FUNCTION("FlyingCameraLogic::init");
    DanglingBaseClassRef<SceneNode> cn = user_object_.cameras.camera(DP_LOC).node;
    if (fly_) {
        flying_key_callback(user_object_, *keys_);
        cn->set_position(user_object_.position, SUCCESSOR_POSE);
        cn->set_rotation(user_object_.angles, SUCCESSOR_POSE);
    } else {
        nofly_key_callback(user_object_, *keys_);
    }
    if (rotate_) {
        DanglingBaseClassRef<SceneNode> on = scene_.get_node(user_object_.obj_node_name, DP_LOC);
        on->set_position(user_object_.obj_position, SUCCESSOR_POSE);
        on->set_rotation(user_object_.obj_angles, SUCCESSOR_POSE);
    }
    return std::nullopt;
}

void FlyingCameraLogic::render_without_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{}

void FlyingCameraLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "FlyingCameraLogic";
}
