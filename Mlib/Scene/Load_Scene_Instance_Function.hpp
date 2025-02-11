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
class IParticleRenderer;
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
class Imposters;
class SupplyDepots;
class RenderingResources;

class LoadSceneInstanceFunction {
public:
    explicit LoadSceneInstanceFunction(RenderableScene& renderable_scene);
    ~LoadSceneInstanceFunction();

protected:
    RenderableScene& renderable_scene;
    ObjectPool& object_pool;
    RenderingResources& rendering_resources;
    SceneNodeResources& scene_node_resources;
    ParticleResources& particle_resources;
    IParticleRenderer& particle_renderer;
    ITrailRenderer& trail_renderer;
    SmokeParticleGenerator& smoke_particle_generator;
    DynamicLights& dynamic_lights;
    VehicleSpawners& vehicle_spawners;
    Players& players;
    Scene& scene;
    DynamicWorld& dynamic_world;
    PhysicsEngine& physics_engine;
    Imposters& imposters;
    SupplyDepots& supply_depots;
    KeyBindings& key_bindings;
    SelectedCameras& selected_cameras;
    const SceneConfig& scene_config;
    RenderLogics& render_logics;
    RenderLogics& scene_render_logics;
    std::function<bool()>& paused;
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
    DeleteNodeMutex& delete_node_mutex;

private:
#ifndef WITHOUT_ALUT
    AudioResourceContextGuard arg0_;
    AudioResourceContextGuard arg1_;
#endif
};

}
