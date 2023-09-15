#pragma once
#include <Mlib/Macro_Executor/Macro_Recorder.hpp>
#include <Mlib/Scene/Json_User_Function.hpp>
#include <atomic>
#include <map>

struct GLFWwindow;

namespace Mlib {

class RenderLogicGallery;
class AssetReferences;
class RenderableScenes;
class NotifyingJsonMacroArguments;
struct SceneConfig;
struct RenderConfig;
class ButtonStates;
class CursorStates;
struct UiFocus;
class RenderLogics;
class ThreadSafeString;
class SurfaceContactDb;
class LayoutConstraints;
struct RealtimeDependentFps;

class LoadScene {
public:
    LoadScene();
    ~LoadScene();
    void operator () (
        const std::list<std::string>* search_path,
        const std::string& script_filename,
        ThreadSafeString& next_scene_filename,
        NotifyingJsonMacroArguments& external_json_macro_arguments,
        std::atomic_size_t& num_renderings,
        RealtimeDependentFps& render_set_fps,
        bool verbose,
        SurfaceContactDb& surface_contact_db,
        SceneConfig& scene_config,
        ButtonStates& button_states,
        CursorStates& cursor_states,
        CursorStates& scroll_wheel_states,
        UiFocus& ui_focus,
        LayoutConstraints& layout_constraints,
#ifndef __ANDROID__
        GLFWwindow& glfw_window,
#endif
        RenderLogicGallery& gallery,
        AssetReferences& asset_references,
        RenderableScenes& renderable_scenes);
    void register_json_user_function(const std::string& key, LoadSceneJsonUserFunction function);
private:
    MacroRecorder macro_file_executor_;
    std::map<std::string, LoadSceneJsonUserFunction> json_user_functions_;
};

}
