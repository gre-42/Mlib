#pragma once
#include <Mlib/Audio/Audio_Resource_Context.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/User_Function.hpp>
#include <atomic>
#include <functional>

namespace Mlib {

struct LoadSceneUserFunctionArgs;
class RenderableScene;
class SceneNodeResources;
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
class SupplyDepots;

class LoadSceneInstanceFunction {
public:
    explicit LoadSceneInstanceFunction(RenderableScene& renderable_scene);
    ~LoadSceneInstanceFunction();
protected:
    SceneNodeResources& scene_node_resources;
    Players& players;
    Scene& scene;
    PhysicsEngine& physics_engine;
    SupplyDepots& supply_depots;
    ButtonPress& button_press;
    CursorStates& cursor_states;
    CursorStates& scroll_wheel_states;
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
    AudioResourceContextGuard arg0_;
    AudioResourceContextGuard arg1_;
};

}
