#pragma once
#include <Mlib/Audio/Audio_Resource_Context.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Memory/Event_Emitter.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Memory/Usage_Counter.hpp>
#include <Mlib/Physics/Advance_Times/Countdown_Physics.hpp>
#include <Mlib/Physics/Bullets/Bullet_Generator.hpp>
#include <Mlib/Physics/Misc/Gravity_Efp.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Smoke_Generation/Contact_Smoke_Generator.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Players/Containers/Vehicle_Spawners.hpp>
#include <Mlib/Players/Game_Logic/Supply_Depots.hpp>
#include <Mlib/Remote/Remote_Site_Id.hpp>
#include <Mlib/Render/Deferred_Instantiator.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Scene/Scene_Particles.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Fifo_Log.hpp>
#include <Mlib/Scene_Graph/Instances/Dynamic_World.hpp>
#include <Mlib/Time/Fps/Dependent_Sleeper.hpp>
#include <Mlib/Time/Fps/Realtime_Sleeper.hpp>
#include <Mlib/Time/Fps/Set_Fps.hpp>

namespace Mlib {

class GameLogic;
class PhysicsScene;
class AssetReferences;
class SceneNodeResources;
class ParticleResources;
class TrailResources;
class ITrailRenderer;
class DynamicLights;
class SurfaceContactDb;
class BulletPropertyDb;
class DynamicLightDb;
class OneShotAudio;
class RemoteScene;
class Translator;
class NotifyingJsonMacroArguments;
class RemoteSites;

class PhysicsIteration;
class PhysicsLoop;

struct SceneConfig;
struct RaceIdentifier;
struct RemoteParams;

enum class ThreadAffinity;
enum class RemoteRole;

class PhysicsScene final: public virtual DanglingBaseClass, public virtual DestructionNotifier {
public:
    PhysicsScene(
        std::string name,
        VariableAndHash<std::string> world,
        std::string rendering_resources_name,
        unsigned int max_anisotropic_filtering_level,
        SceneConfig& scene_config,
        const MacroLineExecutor& macro_line_executor,
        RemoteSites& remote_sites,
        AssetReferences& asset_references,
        SceneNodeResources& scene_node_resources,
        ParticleResources& particle_resources,
        TrailResources& trail_resources,
        SurfaceContactDb& surface_contact_db,
        BulletPropertyDb& bullet_property_db,
        DynamicLightDb& dynamic_light_db,
        size_t max_tracks,
        bool save_playback,
        const RaceIdentifier& race_identfier,
        DependentSleeper& dependent_sleeper,
        UiFocus& ui_focus,
        std::shared_ptr<Translator> translator,
        const std::optional<RemoteParams>& remote_params);
    ~PhysicsScene();

    // Misc
    void start_physics_loop(
        const std::string& thread_name,
        ThreadAffinity thread_affinity);
    void physics_iteration(std::chrono::steady_clock::time_point time);
    void print_physics_engine_search_time() const;
    void plot_physics_triangle_bvh_svg(const std::string& filename, size_t axis0, size_t axis1) const;
    void stop_and_join();
    void shutdown();
    void instantiate_game_logic(std::function<void()> setup_new_round);

    DestructionFunctions on_stop_and_join_;
    DeleteNodeMutex delete_node_mutex_;
    MacroLineExecutor macro_line_executor_;
    DanglingBaseClassRef<RemoteSites> remote_sites_;
    UiFocus& ui_focus_;
    std::string name_;
    const SceneConfig& scene_config_;
    DanglingBaseClassRef<AssetReferences> asset_references_;
    SceneNodeResources& scene_node_resources_;
    ParticleResources& particle_resources_;
    BulletPropertyDb& bullet_property_db_;
    RenderingResources rendering_resources_;
    std::function<bool()> paused_;
    EventEmitter paused_changed_;
    std::unique_ptr<ITrailRenderer> trail_renderer_;
    std::unique_ptr<DynamicLights> dynamic_lights_;
    DynamicWorld dynamic_world_;
    RenderLogics render_logics_;
    PhysicsEngine physics_engine_;
    DeferredInstantiator deferred_instantiator_;
    Scene scene_;
    ObjectPool object_pool_;
    OneShotAudio& one_shot_audio_;
    SceneParticles air_particles_;
    SceneParticles skidmark_particles_;
    SceneParticles sea_spray_particles_;
    ContactSmokeGenerator contact_smoke_generator_;
    BulletGenerator bullet_generator_;
    RealtimeSleeper physics_sleeper_;
    FifoLog fifo_log_{10 * 1000};
    SetFps physics_set_fps_;
    BusyStateProviderGuard busy_state_provider_guard_;
    GravityEfp gefp_;
    std::unique_ptr<PhysicsIteration> physics_iteration_;
    std::unique_ptr<PhysicsLoop> physics_loop_;
    VehicleSpawners vehicle_spawners_;
    Players players_;
    SupplyDepots supply_depots_;
    std::unique_ptr<GameLogic> game_logic_;
    UsageCounter usage_counter_;
    CounterUser remote_counter_user_;
    CountdownPhysics countdown_start_;
    std::shared_ptr<Translator> translator_;
    std::unique_ptr<RemoteScene> remote_scene_;

    AudioResourceContext primary_audio_resource_context_;
    AudioResourceContext secondary_audio_resource_context_;
};

}
