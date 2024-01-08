#pragma once
#include <Mlib/Audio/Audio_Resource_Context.hpp>
#include <Mlib/Images/Ppm_Image.hpp>
#include <Mlib/Physics/Misc/Gravity_Efp.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Iteration.hpp>
#include <Mlib/Physics/Smoke_Generation/Contact_Smoke_Generator.hpp>
#include <Mlib/Physics/Smoke_Generation/Smoke_Particle_Generator.hpp>
#include <Mlib/Players/Advance_Times/Game_Logic.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Players/Containers/Vehicle_Spawners.hpp>
#include <Mlib/Players/Game_Logic/Supply_Depots.hpp>
#include <Mlib/Regex/Misc.hpp>
#include <Mlib/Render/Imposters.hpp>
#include <Mlib/Render/Key_Bindings/Key_Configurations.hpp>
#include <Mlib/Render/Render_Logics/Flying_Camera_Logic.hpp>
#include <Mlib/Render/Render_Logics/Read_Pixels_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Render_Logics/Skybox_Logic.hpp>
#include <Mlib/Render/Render_Logics/Standard_Camera_Logic.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Obj_File_Resource.hpp>
#include <Mlib/Render/Selected_Cameras/Selected_Cameras.hpp>
#include <Mlib/Scene/Load_Scene.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene/Scene_Config.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Fifo_Log.hpp>
#include <Mlib/Time/Fps/Dependent_Sleeper.hpp>
#include <Mlib/Time/Fps/Realtime_Sleeper.hpp>
#include <Mlib/Time/Fps/Set_Fps.hpp>
#include <vector>

#ifndef __ANDROID__
struct GLFWwindow;
#endif

namespace Mlib {

class SceneNodeResources;
class ParticleResources;
class ParticleRenderer;
class SurfaceContactDb;

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

enum class ThreadAffinity;
enum class ClearMode;

struct FocusFilter;

struct SceneConfigResource {
    bool fly;
    bool rotate;
    bool depth_fog;
    bool low_pass;
    bool high_pass;
    bool with_skybox;
    bool with_flying_logic;
    FixedArray<float, 3> background_color;
    ClearMode clear_mode;
};

class RenderableScene: public RenderLogic {
public:
    RenderableScene(
        std::string rendering_resources_name,
        unsigned int max_anisotropic_filtering_level,
        SceneNodeResources& scene_node_resources,
        ParticleResources& particle_resources,
        SurfaceContactDb& surface_contact_db,
        SceneConfig& scene_config,
        ButtonStates& button_states,
        CursorStates& cursor_states,
        CursorStates& scroll_wheel_states,
        UiFocus& ui_focus,
#ifndef __ANDROID__
        GLFWwindow& glfw_window,
#endif
        const SceneConfigResource& config,
        const std::string& level_name,
        size_t max_tracks,
        bool save_playback,
        const RaceIdentifier& race_identfier,
        const std::function<void()>& setup_new_round,
        const FocusFilter& focus_filter,
        DependentSleeper& dependent_sleeper);
    ~RenderableScene();
    RenderableScene(const RenderableScene&) = delete;
    RenderableScene& operator = (const RenderableScene&) = delete;

    // RenderLogic
    virtual void render(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

    // Misc
    void start_physics_loop(
        const std::string& thread_name,
        ThreadAffinity thread_affinity);
    void print_physics_engine_search_time() const;
    void plot_physics_triangle_bvh_svg(const std::string& filename, size_t axis0, size_t axis1) const;
    void stop_and_join();
    void clear();
    void instantiate_audio_listener(
        std::chrono::steady_clock::duration delay,
        std::chrono::steady_clock::duration velocity_dt);

    DeleteNodeMutex delete_node_mutex_;

    SceneNodeResources& scene_node_resources_;
    ParticleResources& particle_resources_;
    RenderingResources rendering_resources_;
    std::unique_ptr<ParticleRenderer> particle_renderer_;
    const SceneConfig& scene_config_;
    PhysicsEngine physics_engine_;
    VehicleSpawners vehicle_spawners_;
    Scene scene_;
    SelectedCameras selected_cameras_;
    FlyingCameraUserClass user_object_;

    SmokeParticleGenerator smoke_particle_generator_;
    ContactSmokeGenerator contact_smoke_generator_;

    std::function<bool()> paused_;
    RealtimeSleeper physics_sleeper_;
    SetFps physics_set_fps_;
    BusyStateProviderGuard busy_state_provider_guard_;
    FifoLog fifo_log_{10 * 1000};
    GravityEfp gefp_;
    PhysicsIteration physics_iteration_;
    std::unique_ptr<PhysicsLoop> physics_loop_;

    StandardCameraLogic standard_camera_logic_;
    SkyboxLogic skybox_logic_;
    std::shared_ptr<StandardRenderLogic> standard_render_logic_;
    std::shared_ptr<FlyingCameraLogic> flying_camera_logic_;
    KeyConfigurations key_configurations_;
    std::shared_ptr<KeyBindings> key_bindings_;
    ReadPixelsLogic read_pixels_logic_;
    std::shared_ptr<DirtmapLogic> dirtmap_logic_;
    std::shared_ptr<MotionInterpolationLogic> motion_interp_logic_;
    std::shared_ptr<PostProcessingLogic> post_processing_logic_;
    std::shared_ptr<FxaaLogic> fxaa_logic_;
    std::shared_ptr<RenderLogics> imposter_render_logics_;
    RenderLogics render_logics_;

    Imposters imposters_;
    Players players_;
    SupplyDepots supply_depots_;
    GameLogic game_logic_;
    std::unique_ptr<AudioListenerUpdater> audio_listener_updater_;

#ifndef WITHOUT_ALUT
    AudioResourceContext primary_audio_resource_context_;
    AudioResourceContext secondary_audio_resource_context_;
#endif
};

}
