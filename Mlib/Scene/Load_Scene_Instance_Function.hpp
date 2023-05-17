#pragma once
#include <Mlib/Audio/Audio_Resource_Context.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <atomic>
#include <functional>

namespace Mlib {

struct LoadSceneUserFunctionArgs;
class RenderableScene;
class SceneNodeResources;
class ParticlesResources;
class SmokeParticleGenerator;
class VehicleSpawners;
class Players;
class Scene;
class PhysicsEngine;
class ButtonPress;
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
class PostProcessingLogic;
class Imposters;
class SupplyDepots;

class LoadSceneInstanceFunction {
public:
    explicit LoadSceneInstanceFunction(RenderableScene& renderable_scene);
    ~LoadSceneInstanceFunction();
protected:
    RenderableScene& renderable_scene;
    SceneNodeResources& scene_node_resources;
    ParticlesResources& particles_resources;
    SmokeParticleGenerator& smoke_particle_generator;
    VehicleSpawners& vehicle_spawners;
    Players& players;
    Scene& scene;
    PhysicsEngine& physics_engine;
    Imposters& imposters;
    SupplyDepots& supply_depots;
    ButtonPress& button_press;
    KeyBindings& key_bindings;
    SelectedCameras& selected_cameras;
    const SceneConfig& scene_config;
    RenderLogics& render_logics;
    std::function<bool()>& paused;
    SetFps& physics_set_fps;
    RenderLogic& scene_logic;
    RenderLogic& read_pixels_logic;
    DirtmapLogic& dirtmap_logic;
    StandardRenderLogic& standard_render_logic;
    PostProcessingLogic& post_processing_logic;
    SkyboxLogic& skybox_logic;
    GameLogic& game_logic;
    BaseLog& base_log;
    DeleteNodeMutex& delete_node_mutex;
    RenderingContext& primary_rendering_context;
    RenderingContext& secondary_rendering_context;
private:
    RenderingContextGuard rrg0_;
    RenderingContextGuard rrg1_;
#ifndef WITHOUT_ALUT
    AudioResourceContextGuard arg0_;
    AudioResourceContextGuard arg1_;
#endif
};

}
