#pragma once
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <atomic>
#include <functional>
#include <optional>
#ifndef WITHOUT_AUDIO
#include <Mlib/Audio/Audio_Resource_Context.hpp>
#endif
#ifndef WITHOUT_GRAPHICS
#include <Mlib/Resource_Context/Rendering_Context.hpp>
#endif

namespace Mlib {

class UserInfo;
template <class... Args>
class EventReceiverDeletionToken;
class BulletPropertyDb;
class MacroLineExecutor;
class PhysicsScene;
class ObjectPool;
class DeferredInstantiator;
class SceneNodeResources;
class AssetReferences;
class ParticleResources;
class ITrailRenderer;
class SmokeParticleGenerator;
class DynamicLights;
class VehicleSpawners;
class OneShotAudio;
class BulletGenerator;
class Players;
class Scene;
class DynamicWorld;
class PhysicsEngine;
class CursorStates;
class SetFps;
class RenderLogic;
class GameLogic;
class BaseLog;
class DirtmapLogic;
class SkyboxLogic;
class StandardRenderLogic;
class AggregateRenderLogic;
class PostProcessingLogic;
class SupplyDepots;
class RenderingResources;
template <class... Args>
class EventEmitter;
class RenderLogics;
class UiFocus;
class CountdownPhysics;
class RemoteScene;
class RemoteSites;
class Translator;
struct SceneParticles;
struct SceneConfig;
struct RenderingContext;
class LateJoinPlayerFactory;

class LoadPhysicsSceneInstanceFunction {
public:
    explicit LoadPhysicsSceneInstanceFunction(
        PhysicsScene& physics_scene,
        const MacroLineExecutor* macro_line_executor = nullptr);
    ~LoadPhysicsSceneInstanceFunction();

protected:
    PhysicsScene& physics_scene;
    ObjectPool& object_pool;
    const MacroLineExecutor& macro_line_executor;
    const std::shared_ptr<Translator>& translator;
    BulletPropertyDb& bullet_property_db;
    #ifndef WITHOUT_GRAPHICS
    DeferredInstantiator& deferred_instantiator;
    RenderLogics& render_logics;
    RenderingResources& rendering_resources;
    #endif
    SceneNodeResources& scene_node_resources;
    AssetReferences& asset_references;
    ParticleResources& particle_resources;
    SceneParticles& air_particles;
    SceneParticles& skidmark_particles;
    SceneParticles& sea_spray_particles;
    ITrailRenderer& trail_renderer;
    DynamicLights& dynamic_lights;
    #ifndef WITHOUT_AUDIO
    OneShotAudio& one_shot_audio;
    #endif
    BulletGenerator& bullet_generator;
    VehicleSpawners& vehicle_spawners;
    Players& players;
    Scene& scene;
    DynamicWorld& dynamic_world;
    PhysicsEngine& physics_engine;
    SupplyDepots& supply_depots;
    const SceneConfig& scene_config;
    std::function<bool()>& paused;
    EventEmitter<>& paused_changed;
    SetFps& physics_set_fps;
    GameLogic* game_logic;
    RemoteScene* remote_scene;
    RemoteSites& remote_sites;
    CountdownPhysics& countdown_start;
    #ifndef WITHOUT_GRAPHICS
    UiFocus& ui_focus;
    #endif
    BaseLog& base_log;
    SafeAtomicRecursiveSharedMutex& delete_node_mutex;
    std::optional<EventReceiverDeletionToken<const UserInfo&>>& on_user_loaded_level_token;
    std::optional<EventReceiverDeletionToken<>>& on_all_users_loaded_level_token;
    std::optional<LateJoinPlayerFactory>& late_join_player_factory;

private:
    #ifndef WITHOUT_AUDIO
    AudioResourceContextGuard arg0_;
    AudioResourceContextGuard arg1_;
    #endif
};

}
