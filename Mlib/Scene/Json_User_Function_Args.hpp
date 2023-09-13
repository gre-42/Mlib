#pragma once
#include <atomic>
#include <functional>
#include <list>
#include <string>

struct GLFWwindow;

namespace Mlib {

class JsonMacroArguments;
class MacroLineExecutor;
class NotifyingJsonMacroArguments;
struct UiFocus;
class RenderLogicGallery;
class RenderableScene;
class RenderableScenes;
class SurfaceContactDb;
class ThreadSafeString;
struct SceneConfig;
class ButtonStates;
class CursorStates;
struct UiFocus;
class LayoutConstraints;
class AssetReferences;
class DependentSleeper;

struct LoadSceneJsonUserFunctionArgs {
    const std::string& name;
    const JsonMacroArguments& arguments;
    const std::function<RenderableScene&()>& renderable_scene;
    const MacroLineExecutor& macro_line_executor;
    NotifyingJsonMacroArguments& external_json_macro_arguments;
    JsonMacroArguments* local_json_macro_arguments;
    SurfaceContactDb& surface_contact_db;
    SceneConfig& scene_config;
    ButtonStates& button_states;
    CursorStates& cursor_states;
    CursorStates& scroll_wheel_states;
    UiFocus& ui_focus;
    LayoutConstraints& layout_constraints;
#ifndef __ANDROID__
    GLFWwindow& glfw_window;
#endif
    std::atomic_size_t& num_renderings;
    DependentSleeper& render_set_fps_sleeper;
    const std::string& script_filename;
    ThreadSafeString& next_scene_filename;
    RenderLogicGallery& gallery;
    AssetReferences& asset_references;
    RenderableScenes& renderable_scenes;
};

}
