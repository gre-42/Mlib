#pragma once
#include <Mlib/OpenGL/Key_Bindings/Lockable_Key_Configurations_Fwd.hpp>
#include <Mlib/OpenGL/Key_Bindings/Lockable_Key_Descriptions_Fwd.hpp>
#include <atomic>
#include <functional>
#include <list>
#include <string>

namespace Mlib {

template <class T>
class VerboseVector;
class JsonMacroArguments;
class MacroLineExecutor;
class NotifyingJsonMacroArguments;
class UiFocuses;
class Users;
class RemoteSites;
class RenderLogicGallery;
class PhysicsScene;
class PhysicsScenes;
class RenderableScene;
class RenderableScenes;
class SurfaceContactDb;
class BulletPropertyDb;
class DynamicLightDb;
class ThreadSafeString;
struct SceneConfig;
class ButtonStates;
class CursorStates;
class ButtonPress;
class LayoutConstraints;
class AssetReferences;
class Translators;
struct RealtimeDependentFps;
class WindowLogic;
class SceneLevelSelector;

struct LoadSceneJsonUserFunctionArgs {
    const std::string& name;
    const JsonMacroArguments& arguments;
    const std::function<PhysicsScene&()>& physics_scene;
    const MacroLineExecutor& macro_line_executor;
    SceneLevelSelector& scene_level_selector;
    NotifyingJsonMacroArguments& external_json_macro_arguments;
    JsonMacroArguments* local_json_macro_arguments;
    SurfaceContactDb& surface_contact_db;
    BulletPropertyDb& bullet_property_db;
    DynamicLightDb& dynamic_light_db;
    SceneConfig& scene_config;
    Users& users;
    RemoteSites& remote_sites;
    const std::string& script_filename;
    ThreadSafeString& next_scene_filename;
    AssetReferences& asset_references;
    Translators& translators;
    PhysicsScenes& physics_scenes;
    #ifndef WITHOUT_GRAPHICS
    LayoutConstraints& layout_constraints;
    std::atomic_size_t& num_renderings;
    RealtimeDependentFps& render_set_fps;
    const std::function<RenderableScene&()>& renderable_scene;
    ButtonStates& button_states;
    CursorStates& cursor_states;
    CursorStates& scroll_wheel_states;
    VerboseVector<ButtonPress>& confirm_button_press;
    LockableKeyConfigurations& key_configurations;
    LockableKeyDescriptions& key_descriptions;
    UiFocuses& ui_focuses;
    RenderLogicGallery& gallery;
    RenderableScenes& renderable_scenes;
    WindowLogic& window_logic;
    #endif
    const std::function<void()>& exit;
};

}
