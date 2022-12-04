#pragma once
#include <atomic>
#include <functional>
#include <list>
#include <string>

struct GLFWwindow;

namespace Mlib {

class MacroLineExecutor;
class SubstitutionMap;
class RegexSubstitutionCache;
struct UiFocus;
class RenderableScene;
class RenderableScenes;
struct FPath;
class SceneNodeResources;
class ThreadSafeString;
struct SceneConfig;
class ButtonStates;
class CursorStates;
struct UiFocus;

struct LoadSceneUserFunctionArgs {
    const std::string& line;
    const std::function<RenderableScene&()>& renderable_scene;
    const std::function<FPath(const std::string&)>& fpath;
    const std::function<std::list<std::string>(const std::string&)>& fpathes;
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
#ifndef __ANDROID__
    GLFWwindow& glfw_window;
#endif
    std::atomic_size_t& num_renderings;
    const std::string& script_filename;
    ThreadSafeString& next_scene_filename;
    RenderableScenes& renderable_scenes;
};

}
