#pragma once
#include <Mlib/Audio/Audio_Resource_Context.hpp>
#include <Mlib/Fps/Set_Fps.hpp>
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/Physics/Misc/Gravity_Efp.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Physics/Physics_Iteration.hpp>
#include <Mlib/Players/Advance_Times/Game_Logic.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Players/Game_Logic/Supply_Depots.hpp>
#include <Mlib/Regex.hpp>
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

class DirtmapLogic;
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

struct FocusFilter;

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
    FixedArray<float, 3> background_color;
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
        size_t max_tracks,
        const std::function<void()>& setup_new_round,
        const FocusFilter& focus_filter);
    ~RenderableScene();
    RenderableScene(const RenderableScene&) = delete;
    RenderableScene& operator = (const RenderableScene&) = delete;
    void start_physics_loop(const std::string& thread_name);
    void print_physics_engine_search_time() const;
    void plot_physics_triangle_bvh_svg(const std::string& filename, size_t axis0, size_t axis1) const;
    void stop_and_join();
    void instantiate_audio_listener();

    DeleteNodeMutex delete_node_mutex_;

    SceneNodeResources& scene_node_resources_;
    const SceneConfig& scene_config_;
    PhysicsEngine physics_engine_;
    Scene scene_;
    SelectedCameras selected_cameras_;
    ButtonStates& button_states_;
    CursorStates& cursor_states_;
    CursorStates& scroll_wheel_states_;
    FlyingCameraUserClass user_object_;

    std::function<bool()> paused_;
    SetFps physics_set_fps_;
    FifoLog fifo_log_{10 * 1000};
    GravityEfp gefp_;
    PhysicsIteration physics_iteration_;
    std::unique_ptr<PhysicsLoop> physics_loop_;

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
    RenderLogics render_logics_;
    Players players_;
    SupplyDepots supply_depots_;
    std::unique_ptr<PodBots> pod_bots_;
    GameLogic game_logic_;
    std::unique_ptr<AudioListenerUpdater> audio_listener_updater_;

    RenderingContext primary_rendering_context_;
    RenderingContext secondary_rendering_context_;
    AudioResourceContext primary_audio_resource_context_;
    AudioResourceContext secondary_audio_resource_context_;
};

}
