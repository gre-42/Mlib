#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Fullscreen_Callback.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>

struct GLFWwindow;

namespace Mlib {

class Scene;
class SelectedCameras;
class SetFps;
struct WindowPosition;
class ButtonStates;
class CursorStates;
enum class BoolRenderOption;
class DeleteNodeMutex;
class Focuses;

class FlyingCameraUserClass {
public:
#ifndef __ANDROID__
    WindowPosition window_position;
#endif
    ButtonStates& button_states;
    CursorStates& cursor_states;
    CursorStates& scroll_wheel_states;
    SelectedCameras& cameras;
    Focuses& focuses;
    BoolRenderOption& wire_frame;
    BoolRenderOption& depth_test;
    BoolRenderOption& cull_faces;
    DeleteNodeMutex& delete_node_mutex;
    SetFps* physics_set_fps;
    FixedArray<double, 3> position;
    FixedArray<float, 3> angles;
    FixedArray<double, 3> obj_position;
    FixedArray<float, 3> obj_angles;
    std::string obj_node_name = "obj";
};

class FlyingCameraLogic: public RenderLogic {
public:
    explicit FlyingCameraLogic(
#ifndef __ANDROID__
        GLFWwindow* window,
#endif
        const ButtonStates& button_states,
        const Scene& scene,
        FlyingCameraUserClass& user_object,
        bool fly,
        bool rotate);
    ~FlyingCameraLogic();

    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    const Scene& scene_;
    FlyingCameraUserClass& user_object_;
    ButtonPress button_press_;
    bool fly_;
    bool rotate_;
#ifndef __ANDROID__
    GLFWwindow* window_;
#endif
};

}
