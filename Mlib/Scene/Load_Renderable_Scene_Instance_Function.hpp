#pragma once
#include <Mlib/Audio/Audio_Resource_Context.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <atomic>
#include <functional>

namespace Mlib {

struct LoadSceneUserFunctionArgs;
class RenderableScene;
class ObjectPool;
class SceneNodeResources;
class ParticleResources;
struct SceneParticles;
class ITrailRenderer;
class SmokeParticleGenerator;
class DynamicLights;
class VehicleSpawners;
class Players;
class Scene;
class DynamicWorld;
class PhysicsEngine;
class CursorStates;
class KeyBindings;
class SelectedCameras;
struct SceneConfig;
class RenderLogics;
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
class DeferredInstantiator;
class SupplyDepots;
class RenderingResources;
class EventEmitter;
class UiFocus;
class CountdownPhysics;

class LoadRenderableSceneInstanceFunction {
public:
    explicit LoadRenderableSceneInstanceFunction(RenderableScene& renderable_scene);
    ~LoadRenderableSceneInstanceFunction();

protected:
    RenderableScene& renderable_scene;
    ObjectPool& object_pool;
    RenderingResources& rendering_resources;
    SceneNodeResources& scene_node_resources;
    ParticleResources& particle_resources;
    SceneParticles& air_particles;
    SceneParticles& skidmark_particles;
    SceneParticles& sea_spray_particles;
    ITrailRenderer& trail_renderer;
    DynamicLights& dynamic_lights;
    VehicleSpawners& vehicle_spawners;
    Players& players;
    Scene& scene;
    DynamicWorld& dynamic_world;
    PhysicsEngine& physics_engine;
    DeferredInstantiator& deferred_instantiator;
    SupplyDepots& supply_depots;
    KeyBindings& key_bindings;
    SelectedCameras& selected_cameras;
    const SceneConfig& scene_config;
    RenderLogics& render_logics;
    RenderLogics& scene_render_logics;
    std::function<bool()>& paused;
    EventEmitter& paused_changed;
    SetFps& physics_set_fps;
    RenderLogic& scene_logic;
    RenderLogic& read_pixels_logic;
    DirtmapLogic& dirtmap_logic;
    StandardRenderLogic& standard_render_logic;
    AggregateRenderLogic& aggregate_render_logic;
    PostProcessingLogic& post_processing_logic;
    SkyboxLogic& skybox_logic;
    GameLogic* game_logic;
    BaseLog& base_log;
    CountdownPhysics& countdown_start;
    UiFocus& ui_focus;
    DeleteNodeMutex& delete_node_mutex;

private:
    AudioResourceContextGuard arg0_;
    AudioResourceContextGuard arg1_;
};

}
