#pragma once
#include <Mlib/Macro_Recorder.hpp>
#include <map>
#include <memory>
#include <mutex>

struct GLFWwindow;

namespace Mlib {

class RenderableScene;
class SubstitutionMap;

class SceneNodeResources;
struct SceneConfig;
struct RenderConfig;
class ButtonStates;
class CursorStates;
struct UiFocus;
class RenderLogics;

class LoadScene {
public:
    void operator () (
        const std::string& working_directory,
        const std::string& script_filename,
        std::string& next_scene_filename,
        SubstitutionMap& external_substitutions,
        size_t& num_renderings,
        bool verbose,
        RegexSubstitutionCache& rsc,
        SceneNodeResources& scene_node_resources,
        SceneConfig& scene_config,
        ButtonStates& button_states,
        CursorStates& cursor_states,
        CursorStates& scroll_wheel_states,
        UiFocus& ui_focus,
        GLFWwindow* window,
        std::map<std::string, std::shared_ptr<RenderableScene>>& renderable_scenes);
private:
    MacroRecorder macro_file_executor_;
};

}
