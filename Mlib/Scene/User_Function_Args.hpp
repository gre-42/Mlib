#pragma once
#include <functional>
#include <string>

struct GLFWwindow;

namespace Mlib {

class MacroLineExecutor;
class SubstitutionMap;
class RegexSubstitutionCache;
struct UiFocus;
class RenderableScene;
struct FPath;
class SceneNodeResources;
struct SceneConfig;

struct SceneConfig;
class ButtonStates;
class CursorStates;
struct UiFocus;

struct LoadSceneUserFunctionArgs {
    const std::string& line;
    const std::function<RenderableScene&()>& renderable_scene;
    const std::function<FPath(const std::string&)>& fpath;
    const MacroLineExecutor& macro_line_executor;
    SubstitutionMap& external_substitutions;
    SubstitutionMap* local_substitutions;
    RegexSubstitutionCache& rsc;
    SceneNodeResources& scene_node_resources;
    SceneConfig& scene_config;
    ButtonStates& button_states;
    CursorStates& cursor_states;
    CursorStates& scroll_wheel_states;
    UiFocus& ui_focus;
    GLFWwindow* window;
    size_t& num_renderings;
    const std::string& script_filename;
    std::string& next_scene_filename;
    std::map<std::string, RenderableScene>& renderable_scenes;
};

}
