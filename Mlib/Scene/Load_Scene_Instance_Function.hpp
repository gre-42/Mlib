#pragma once
#include <Mlib/Scene/User_Function.hpp>
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

class LoadSceneInstanceFunction {
public:
    explicit LoadSceneInstanceFunction(RenderableScene& renderable_scene);
protected:
    SceneNodeResources& scene_node_resources;
    Players& players;
    Scene& scene;
    PhysicsEngine& physics_engine;
    ButtonPress& button_press;
    CursorStates& cursor_states;
    CursorStates& scroll_wheel_states;
    KeyBindings& key_bindings;
    SelectedCameras& selected_cameras;
    const SceneConfig& scene_config;
    RenderLogics& render_logics;
    SetFps& physics_set_fps;
    RenderLogic& scene_logic;
    RenderLogic& read_pixels_logic;
    RenderLogic& dirtmap_logic;
    RenderLogic& post_processing_logic;
    RenderLogic& skybox_logic;
    GameLogic& game_logic;
    BaseLog& base_log;
    DeleteNodeMutex& delete_node_mutex;
};

}
