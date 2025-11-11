#pragma once
#include <Mlib/Audio/Audio_Resource_Context.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <atomic>
#include <functional>

namespace Mlib {

class PhysicsScene;
class ObjectPool;
class DeferredInstantiator;
class SceneNodeResources;
class AssetReferences;
class ParticleResources;
struct SceneParticles;
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
struct SceneConfig;
class SetFps;
class RenderLogic;
class GameLogic;
class BaseLog;
class DeleteNodeMutex;
struct RenderingContext;
class DirtmapLogic;
class SkyboxLogic;
class StandardRenderLogic;
class AggregateRenderLogic;
class PostProcessingLogic;
class SupplyDepots;
class RenderingResources;
class EventEmitter;
class RenderLogics;
class UiFocus;
class CountdownPhysics;
class RemoteScene;
class RemoteSites;

class LoadPhysicsSceneInstanceFunction {
public:
    explicit LoadPhysicsSceneInstanceFunction(PhysicsScene& physics_scene);
    ~LoadPhysicsSceneInstanceFunction();

protected:
    PhysicsScene& physics_scene;
    ObjectPool& object_pool;
    DeferredInstantiator& deferred_instantiator;
    RenderLogics& render_logics;
    RenderingResources& rendering_resources;
    SceneNodeResources& scene_node_resources;
    AssetReferences& asset_references;
    ParticleResources& particle_resources;
    SceneParticles& air_particles;
    SceneParticles& skidmark_particles;
    SceneParticles& sea_spray_particles;
    ITrailRenderer& trail_renderer;
    DynamicLights& dynamic_lights;
    OneShotAudio& one_shot_audio;
    BulletGenerator& bullet_generator;
    VehicleSpawners& vehicle_spawners;
    Players& players;
    Scene& scene;
    DynamicWorld& dynamic_world;
    PhysicsEngine& physics_engine;
    SupplyDepots& supply_depots;
    const SceneConfig& scene_config;
    std::function<bool()>& paused;
    EventEmitter& paused_changed;
    SetFps& physics_set_fps;
    GameLogic* game_logic;
    RemoteScene* remote_scene;
    RemoteSites& remote_sites;
    CountdownPhysics& countdown_start;
    UiFocus& ui_focus;
    BaseLog& base_log;
    DeleteNodeMutex& delete_node_mutex;

private:
    AudioResourceContextGuard arg0_;
    AudioResourceContextGuard arg1_;
};

}
