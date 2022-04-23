#pragma once
#include <Mlib/Audio/Audio_Resource_Context.hpp>
#include <Mlib/Fps/Set_Fps.hpp>
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/Physics/Misc/Gravity_Efp.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Physics/Physics_Iteration.hpp>
#include <Mlib/Players/Advance_Times/Game_Logic.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex.hpp>
#include <Mlib/Render/Aggregate_Array_Renderer.hpp>
#include <Mlib/Render/Array_Instances_Renderer.hpp>
#include <Mlib/Render/Render_Logics/Flying_Camera_Logic.hpp>
#include <Mlib/Render/Render_Logics/Read_Pixels_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Render_Logics/Skybox_Logic.hpp>
#include <Mlib/Render/Render_Logics/Standard_Camera_Logic.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Obj_File_Resource.hpp>
#include <Mlib/Render/Selected_Cameras.hpp>
#include <Mlib/Scene/Load_Scene.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene/Scene_Config.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Fifo_Log.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <vector>

namespace Mlib {

class PostProcessingLogic;
class FxaaLogic;
class MotionInterpolationLogic;
class FlyingCameraUserClass;
class StandardRenderLogic;

class AudioListenerUpdater;
class PhysicsLoop;
class ButtonStates;
class CursorStates;
class PodBots;

enum class ClearMode;

struct SceneConfigResource {
    bool fly;
    bool rotate;
    bool print_gamepad_buttons;
    bool depth_fog;
    bool low_pass;
    bool high_pass;
    bool with_dirtmap;
    bool with_skybox;
    bool with_flying_logic;
    bool with_pod_bot;
    ClearMode clear_mode;
};

class RenderableScene {
public:
    RenderableScene(
        SceneNodeResources& scene_node_resources,
        SceneConfig& scene_config,
        ButtonStates& button_states,
        CursorStates& cursor_states,
        CursorStates& scroll_wheel_states,
        UiFocus& ui_focus,
        GLFWwindow* window,
        const SceneConfigResource& config,
        const std::string& level_name,
        size_t max_tracks);
    ~RenderableScene();
    void start_physics_loop(const std::string& thread_name);
    void print_physics_engine_search_time() const;
    void plot_physics_triangle_bvh_svg(const std::string& filename, size_t axis0, size_t axis1) const;
    void stop_and_join();
    void instantiate_audio_listener();
    SceneNodeResources& scene_node_resources_;
    std::shared_ptr<AggregateRenderer> small_sorted_aggregate_renderer_;
    std::shared_ptr<InstancesRenderer> small_instances_renderer_;
    AggregateArrayRenderer large_aggregate_array_renderer_;
    ArrayInstancesRenderer large_instances_renderer_;
    PhysicsEngine physics_engine_;
    Scene scene_;
    SelectedCameras selected_cameras_;
    ButtonStates& button_states_;
    CursorStates& cursor_states_;
    CursorStates& scroll_wheel_states_;
    FlyingCameraUserClass user_object_;
    std::atomic_bool audio_paused_{false};
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
    std::shared_ptr<FxaaLogic> fxaa_logic_;
    DeleteNodeMutex delete_node_mutex_;
    RenderLogics render_logics_;
    Players players_;
    std::unique_ptr<PodBots> pod_bots_;
    GameLogic game_logic_;
    std::unique_ptr<AudioListenerUpdater> audio_listener_updater_;

    const SceneConfig& scene_config_;
    PhysicsIteration physics_iteration_;
    std::unique_ptr<PhysicsLoop> physics_loop_;
    RenderingContext primary_rendering_context_;
    RenderingContext secondary_rendering_context_;
    AudioResourceContext primary_audio_resource_context_;
    AudioResourceContext secondary_audio_resource_context_;
};

}
