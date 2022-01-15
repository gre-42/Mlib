#pragma once
#include <functional>
#include <string>

namespace Mlib {

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
class RenderLogic;
class RenderLogic;
class RenderLogic;
class RenderLogic;
class GameLogic;
class BaseLog;
class DeleteNodeMutex;
struct FPath;
class MacroLineExecutor;
class SubstitutionMap;
class RegexSubstitutionCache;

class LoadSceneInstanceFunction {
public:
    typedef std::function<bool(
        const std::string& line,
        const std::function<RenderableScene&()>& renderable_scene,
        const std::function<FPath(const std::string&)>& fpath,
        const MacroLineExecutor& macro_line_executor,
        SubstitutionMap& external_substitutions,
        SubstitutionMap* local_substitutions,
        RegexSubstitutionCache& rsc)> UserFunction;
    explicit LoadSceneInstanceFunction(RenderableScene& renderable_scene);
protected:
    SceneNodeResources& scene_node_resources;
    Players& players;
    Scene& scene;
    PhysicsEngine& physics_engine;
    ButtonPress& button_press;
    CursorStates& cursor_states;
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
