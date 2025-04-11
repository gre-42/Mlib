#pragma once
#include <Mlib/Render/Key_Bindings/Lockable_Key_Configurations_Fwd.hpp>
#include <Mlib/Render/Key_Bindings/Lockable_Key_Descriptions_Fwd.hpp>
#include <atomic>
#include <functional>
#include <list>
#include <string>

namespace Mlib {

class JsonMacroArguments;
class MacroLineExecutor;
class NotifyingJsonMacroArguments;
class UiFocus;
class RenderLogicGallery;
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

struct LoadSceneJsonUserFunctionArgs {
    const std::string& name;
    const JsonMacroArguments& arguments;
    const std::function<RenderableScene&()>& renderable_scene;
    const MacroLineExecutor& macro_line_executor;
    NotifyingJsonMacroArguments& external_json_macro_arguments;
    JsonMacroArguments* local_json_macro_arguments;
    SurfaceContactDb& surface_contact_db;
    BulletPropertyDb& bullet_property_db;
    DynamicLightDb& dynamic_light_db;
    SceneConfig& scene_config;
    ButtonStates& button_states;
    CursorStates& cursor_states;
    CursorStates& scroll_wheel_states;
    ButtonPress& confirm_button_press;
    LockableKeyConfigurations& key_configurations;
    LockableKeyDescriptions& key_descriptions;
    UiFocus& ui_focus;
    LayoutConstraints& layout_constraints;
    std::atomic_size_t& num_renderings;
    RealtimeDependentFps& render_set_fps;
    const std::string& script_filename;
    ThreadSafeString& next_scene_filename;
    RenderLogicGallery& gallery;
    AssetReferences& asset_references;
    Translators& translators;
    RenderableScenes& renderable_scenes;
    const std::function<void()>& exit;
};

}
