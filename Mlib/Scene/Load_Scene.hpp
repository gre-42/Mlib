#pragma once
#include <Mlib/Macro_Executor/Macro_Recorder.hpp>
#include <Mlib/Scene/User_Function.hpp>
#include <atomic>
#include <map>

struct GLFWwindow;

namespace Mlib {

class RenderableScenes;
class SubstitutionMap;
class SceneNodeResources;
struct SceneConfig;
struct RenderConfig;
class ButtonStates;
class CursorStates;
struct TapButtonsStates;
struct UiFocus;
class RenderLogics;
class ThreadSafeString;

class LoadScene {
public:
    LoadScene();
    ~LoadScene();
    void operator () (
        const std::list<std::string>& search_path,
        const std::string& script_filename,
        ThreadSafeString& next_scene_filename,
        SubstitutionMap& external_substitutions,
        std::atomic_size_t& num_renderings,
        bool verbose,
        RegexSubstitutionCache& rsc,
        SceneNodeResources& scene_node_resources,
        SceneConfig& scene_config,
        ButtonStates& button_states,
        CursorStates& cursor_states,
        CursorStates& scroll_wheel_states,
        TapButtonsStates& tap_buttons_states,
        UiFocus& ui_focus,
#ifndef __ANDROID__
        GLFWwindow& glfw_window,
#endif
        RenderableScenes& renderable_scenes);
private:
    MacroRecorder macro_file_executor_;
    std::list<LoadSceneUserFunction> user_functions_;
};

}
