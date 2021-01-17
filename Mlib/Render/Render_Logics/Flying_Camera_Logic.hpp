#pragma once
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Fullscreen_Callback.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>

namespace Mlib {

class Scene;
struct SelectedCameras;
class SetFps;
class ButtonStates;

class FlyingCameraUserClass: public BaseUserObject {
public:
    ButtonStates& button_states;
    SelectedCameras& cameras;
    std::list<Focus>& focus;
    SetFps* physics_set_fps;
    FixedArray<float, 3> position;
    FixedArray<float, 3> angles;
    FixedArray<float, 3> obj_position;
    FixedArray<float, 3> obj_angles;
    std::string obj_node_name = "obj";
};

class FlyingCameraLogic: public RenderLogic {
public:
    explicit FlyingCameraLogic(
        GLFWwindow* window,
        const ButtonStates& button_states,
        const Scene& scene,
        FlyingCameraUserClass& user_object,
        bool fly,
        bool rotate);

    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
private:
    const Scene& scene_;
    FlyingCameraUserClass& user_object_;
    ButtonPress button_press_;
    bool fly_;
    bool rotate_;
};

}
