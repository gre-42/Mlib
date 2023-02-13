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
class RenderLogicGallery;
class RenderableScene;
class RenderableScenes;
struct FPath;
class SceneNodeResources;
class SurfaceContactDb;
class ThreadSafeString;
struct SceneConfig;
class ButtonStates;
class CursorStates;
struct UiFocus;
class LayoutConstraints;
class AssetReferences;

struct LoadSceneUserFunctionArgs {
    const std::string& line;
    const std::function<RenderableScene&()>& renderable_scene;
    const std::function<std::string(const std::string&)>& spath;
    const std::function<FPath(const std::string&)>& fpath;
    const std::function<std::list<std::string>(const std::string&)>& fpathes;
    const MacroLineExecutor& macro_line_executor;
    SubstitutionMap& external_substitutions;
    SubstitutionMap* local_substitutions;
    RegexSubstitutionCache& rsc;
    SceneNodeResources& scene_node_resources;
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
    const std::string& script_filename;
    ThreadSafeString& next_scene_filename;
    RenderLogicGallery& gallery;
    AssetReferences& asset_references;
    RenderableScenes& renderable_scenes;
};

}
