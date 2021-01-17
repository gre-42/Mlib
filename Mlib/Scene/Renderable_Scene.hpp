#pragma once
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/Physics/Advance_Times/Game_Logic.hpp>
#include <Mlib/Physics/Containers/Players.hpp>
#include <Mlib/Physics/Misc/Gravity_Efp.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Physics/Physics_Loop.hpp>
#include <Mlib/Regex.hpp>
#include <Mlib/Render/Aggregate_Array_Renderer.hpp>
#include <Mlib/Render/Array_Instances_Renderer.hpp>
#include <Mlib/Render/Render_Logics/Dirtmap_Logic.hpp>
#include <Mlib/Render/Render_Logics/Flying_Camera_Logic.hpp>
#include <Mlib/Render/Render_Logics/Motion_Interpolation_Logic.hpp>
#include <Mlib/Render/Render_Logics/Post_Processing_Logic.hpp>
#include <Mlib/Render/Render_Logics/Read_Pixels_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Render_Logics/Skybox_Logic.hpp>
#include <Mlib/Render/Render_Logics/Standard_Camera_Logic.hpp>
#include <Mlib/Render/Render_Logics/Standard_Render_Logic.hpp>
#include <Mlib/Render/Renderables/Renderable_Obj_File.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Selected_Cameras.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Scene/Load_Scene.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene/Scene_Config.hpp>
#include <Mlib/Scene_Graph/Fifo_Log.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Set_Fps.hpp>
#include <Mlib/String.hpp>
#include <Mlib/Time_Guard.hpp>
#include <vector>

namespace Mlib {

struct RenderableSceneConfig {
    bool fly;
    bool rotate;
    bool print_gamepad_buttons;
    bool depth_fog;
    bool low_pass;
    bool high_pass;
    bool vfx;
    bool with_dirtmap;
    bool with_skybox;
};

class RenderableScene: public RenderLogic {
public:
    RenderableScene(
        SceneNodeResources& scene_node_resources,
        RenderingResources& rendering_resources,
        const SceneConfig& scene_config,
        ButtonStates& button_states,
        UiFocus& ui_focus,
        std::map<std::string, size_t>& selection_ids,
        GLFWwindow* window,
        RenderLogics& render_logics,
        const RenderableSceneConfig& config,
        std::recursive_mutex& mutex);
    void start_physics_loop();
    void print_physics_engine_search_time() const;
    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    void stop_and_join();
    SceneNodeResources& scene_node_resources_;
    RenderingResources& rendering_resources_;
    SmallSortedAggregateRendererGuard small_sorted_aggregate_renderer_guard_;
    SmallInstancesRendererGuard small_instances_renderer_guard_;
    AggregateArrayRenderer large_aggregate_array_renderer_;
    ArrayInstancesRenderer large_instances_renderer_;
    PhysicsEngine physics_engine_;
    Scene scene_;
    SelectedCameras selected_cameras_;
    UiFocus& ui_focus_;
    std::map<std::string, size_t>& selection_ids_;
    ButtonStates& button_states_;
    FlyingCameraUserClass user_object_;
    SetFps physics_set_fps_{"Physics FPS: "};
    FifoLog fifo_log_{10 * 1000};
    GravityEfp gefp_;
    StandardCameraLogic standard_camera_logic_;
    SkyboxLogic skybox_logic_;
    std::shared_ptr<StandardRenderLogic> standard_render_logic_;
    GLFWwindow* window_;
    std::shared_ptr<FlyingCameraLogic> flying_camera_logic_;
    ButtonPress button_press_;
    std::shared_ptr<KeyBindings> key_bindings_;
    ReadPixelsLogic read_pixels_logic_;
    std::shared_ptr<DirtmapLogic> dirtmap_logic_;
    std::shared_ptr<MotionInterpolationLogic> motion_interp_logic_;
    std::shared_ptr<PostProcessingLogic> post_processing_logic_;
    std::recursive_mutex& mutex_;
    RenderLogics& render_logics_;
    Players players_;
    GameLogic game_logic_;

    std::unique_ptr<PhysicsLoop> physics_loop_;
    const SceneConfig& scene_config_;
};

}
