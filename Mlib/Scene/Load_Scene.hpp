#pragma once
#include <Mlib/Macro_Executor/Macro_Recorder.hpp>
#include <Mlib/Scene/Json_User_Function.hpp>
#include <Mlib/Scene/Load_Scene_User_Function.hpp>
#include <atomic>
#include <map>

struct GLFWwindow;

namespace Mlib {

class RenderLogicGallery;
class AssetReferences;
class RenderableScenes;
class NotifyingSubstitutionMap;
class SceneNodeResources;
struct SceneConfig;
struct RenderConfig;
class ButtonStates;
class CursorStates;
struct UiFocus;
class RenderLogics;
class ThreadSafeString;
class SurfaceContactDb;
class LayoutConstraints;

class LoadScene {
public:
    LoadScene();
    ~LoadScene();
    void operator () (
        const std::list<std::string>* search_path,
        const std::string& script_filename,
        ThreadSafeString& next_scene_filename,
        NotifyingSubstitutionMap& external_substitutions,
        std::atomic_size_t& num_renderings,
        bool verbose,
        SceneNodeResources& scene_node_resources,
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
    void register_user_function(const std::string& key, LoadSceneUserFunction function);
private:
    MacroRecorder macro_file_executor_;
    std::map<std::string, LoadSceneJsonUserFunction> json_user_functions_;
    std::map<std::string, LoadSceneUserFunction> user_functions_;
};

}
