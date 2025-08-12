#pragma once
#include <Mlib/Audio/Audio_Resource_Context.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Memory/Event_Emitter.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Memory/Usage_Counter.hpp>
#include <Mlib/Physics/Misc/Gravity_Efp.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Iteration.hpp>
#include <Mlib/Physics/Smoke_Generation/Contact_Smoke_Generator.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Players/Containers/Users.hpp>
#include <Mlib/Players/Containers/Vehicle_Spawners.hpp>
#include <Mlib/Players/Game_Logic/Supply_Depots.hpp>
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
class SceneNodeResources;
class ParticleResources;
class TrailResources;
class ITrailRenderer;
class DynamicLights;
class SurfaceContactDb;
class DynamicLightDb;
class OneShotAudio;

class PhysicsLoop;

struct SceneConfig;
struct RaceIdentifier;

class Translator;

enum class ThreadAffinity;

class PhysicsScene: public DanglingBaseClass {
public:
    PhysicsScene(
        std::string name,
        VariableAndHash<std::string> world,
        std::string rendering_resources_name,
        unsigned int max_anisotropic_filtering_level,
        SceneConfig& scene_config,
        SceneNodeResources& scene_node_resources,
        ParticleResources& particle_resources,
        TrailResources& trail_resources,
        SurfaceContactDb& surface_contact_db,
        DynamicLightDb& dynamic_light_db,
        size_t max_tracks,
        bool save_playback,
        const RaceIdentifier& race_identfier,
        DependentSleeper& dependent_sleeper,
        UiFocus& ui_focus,
        std::shared_ptr<Translator> translator);
    ~PhysicsScene();

    // Misc
    void start_physics_loop(
        const std::string& thread_name,
        ThreadAffinity thread_affinity);
    void print_physics_engine_search_time() const;
    void plot_physics_triangle_bvh_svg(const std::string& filename, size_t axis0, size_t axis1) const;
    void stop_and_join();
    void clear();
    void instantiate_game_logic(std::function<void()> setup_new_round);

    DestructionFunctions on_stop_and_join_;
    DestructionFunctions on_clear_;
    DeleteNodeMutex delete_node_mutex_;
    ObjectPool object_pool_;
    UiFocus& ui_focus_;
    std::string name_;
    const SceneConfig& scene_config_;
    SceneNodeResources& scene_node_resources_;
    ParticleResources& particle_resources_;
    RenderingResources rendering_resources_;
    std::unique_ptr<ITrailRenderer> trail_renderer_;
    std::unique_ptr<DynamicLights> dynamic_lights_;
    DynamicWorld dynamic_world_;
    RenderLogics render_logics_;
    PhysicsEngine physics_engine_;
    DeferredInstantiator deferred_instantiator_;
    Scene scene_;
    SceneParticles air_particles_;
    SceneParticles skidmark_particles_;
    SceneParticles sea_spray_particles_;
    ContactSmokeGenerator contact_smoke_generator_;
    std::function<bool()> paused_;
    EventEmitter paused_changed_;
    RealtimeSleeper physics_sleeper_;
    FifoLog fifo_log_{10 * 1000};
    SetFps physics_set_fps_;
    BusyStateProviderGuard busy_state_provider_guard_;
    GravityEfp gefp_;
    PhysicsIteration physics_iteration_;
    std::unique_ptr<PhysicsLoop> physics_loop_;
    VehicleSpawners vehicle_spawners_;
    Players players_;
    SupplyDepots supply_depots_;
    std::unique_ptr<GameLogic> game_logic_;
    Users users_;
    UsageCounter usage_counter_;

    std::unique_ptr<OneShotAudio> one_shot_audio_;

    AudioResourceContext primary_audio_resource_context_;
    AudioResourceContext secondary_audio_resource_context_;
};

}
