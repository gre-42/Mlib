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

class FlyingCameraUserClass: public BaseUserObject {
public:
    SelectedCameras& cameras;
    Focus& focus;
    SetFps* physics_set_fps;
    float position_y;
    float position_z;
    float angle_x;
    float angle_y;
    float obj_angle_x;
    float obj_angle_y;
    FixedArray<float, 3> obj_position;
    std::string obj_node_name = "obj";
};

class FlyingCameraLogic: public RenderLogic {
public:
    explicit FlyingCameraLogic(
        GLFWwindow* window,
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
    virtual float near_plane() const override;
    virtual float far_plane() const override;
    virtual const FixedArray<float, 4, 4>& vp() const override;
    virtual bool requires_postprocessing() const override;
private:
    const Scene& scene_;
    FlyingCameraUserClass& user_object_;
    GLFWwindow* window_;
    ButtonPress button_press_;
    bool fly_;
    bool rotate_;
};

}
