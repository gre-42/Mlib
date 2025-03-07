#pragma once
#include <Mlib/Macro_Executor/Macro_Recorder.hpp>
#include <atomic>
#include <functional>

namespace Mlib {

class RenderLogicGallery;
class AssetReferences;
class Translators;
class RenderableScenes;
class NotifyingJsonMacroArguments;
struct SceneConfig;
struct RenderConfig;
class ButtonStates;
class CursorStates;
class ButtonPress;
class KeyConfigurations;
class KeyDescriptions;
class UiFocus;
class RenderLogics;
class ThreadSafeString;
class SurfaceContactDb;
class BulletPropertyDb;
class DynamicLightDb;
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
        BulletPropertyDb& bullet_property_db,
        DynamicLightDb& dynamic_light_db,
        SceneConfig& scene_config,
        ButtonStates& button_states,
        CursorStates& cursor_states,
        CursorStates& scroll_wheel_states,
        ButtonPress& confirm_button_press,
        KeyConfigurations& key_configurations,
        KeyDescriptions& key_descriptions,
        UiFocus& ui_focus,
        LayoutConstraints& layout_constraints,
        RenderLogicGallery& gallery,
        AssetReferences& asset_references,
        Translators& translators,
        RenderableScenes& renderable_scenes,
        const std::function<void()>& exit);
private:
    MacroRecorder macro_file_executor_;
};

}
